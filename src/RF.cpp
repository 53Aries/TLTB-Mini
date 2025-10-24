#include "RF.hpp"
#include <Arduino.h>
#include "pins.hpp"
#include "relays.hpp"
#include <Preferences.h>
#include <RCSwitch.h>
#include "buzzer.hpp"

#ifndef PIN_RF_DATA
#  error "Define PIN_RF_DATA in pins.hpp for SYN480R DATA input"
#endif

namespace {
  // Tunables
  constexpr uint32_t RF_COOLDOWN_MS        = 600;  // suppress repeats after a trigger
  constexpr uint32_t BURST_GAP_MS          = 100;  // gap indicating end of a button-burst
  constexpr uint32_t MAX_BURST_MS          = 240;  // finalize even if still noisy after this window

  uint32_t g_last_activity_ms = 0;

  struct Learned { 
    uint32_t sig; 
    uint32_t sum; 
    uint16_t len; 
    uint8_t relay; 
  };
  Learned g_learn[6];

  Preferences g_prefs;
  int8_t activeRelay = -1; // -1 = none on

  // Deduplicate repeated frames from held buttons
  // Burst aggregator and trigger suppression
  struct VoteAgg {
    bool active;
    uint32_t lastMs;
    uint32_t startMs;
    uint8_t votes[6];
    uint32_t bestScore[6];
    uint8_t coarseVotes[6];
    bool anyEv;
  };
  VoteAgg g_agg = {false, 0, 0, {0,0,0,0,0,0}, 
                   {0xFFFFFFFFu,0xFFFFFFFFu,0xFFFFFFFFu,0xFFFFFFFFu,0xFFFFFFFFu,0xFFFFFFFFu}, 
                   {0,0,0,0,0,0}, false};
  uint32_t g_block_until_ms = 0;

  // Learning state for async operation
  struct {
    bool active = false;
    int channel = -1;
    uint32_t startTime = 0;
    uint32_t deadline = 0;
    uint32_t lastSig = 0;
    uint32_t lastSum = 0;
    uint16_t lastLen = 0;
    uint32_t lastAt = 0;
    bool success = false;
    bool timeout = false;
  } g_learning;

  // Forward declaration for burst finalizer
  void handleTrigger(uint8_t rindex);

  static inline void aggReset() {
    g_agg.active = false;
    g_agg.lastMs = 0;
    g_agg.startMs = 0;
    g_agg.anyEv = false;
    for (int i=0;i<6;++i){ 
      g_agg.votes[i]=0; 
      g_agg.coarseVotes[i]=0; 
      g_agg.bestScore[i]=0xFFFFFFFFu; 
    }
  }

  static void finalizeBurst() {
    if (!g_agg.active) return;
    int winner = -1; 
    uint8_t bestVotes = 0; 
    uint32_t bestScore = 0xFFFFFFFFu;
    
    if (g_agg.anyEv) {
      // choose winner from EV votes first
      for (int i=0;i<6;++i){
        uint8_t v = g_agg.votes[i];
        if (!v) continue;
        uint32_t sc = g_agg.bestScore[i];
        if (v > bestVotes || (v == bestVotes && sc < bestScore)) {
          bestVotes = v; 
          bestScore = sc; 
          winner = i;
        }
      }
    } else {
      // fallback: use coarse votes only if unambiguous (single non-zero bucket)
      int nz = 0; 
      int idx = -1;
      for (int i=0;i<6;++i){ 
        if (g_agg.coarseVotes[i]) { 
          nz++; 
          idx = i; 
        } 
      }
      if (nz == 1 && idx >= 0) winner = idx;
    }
    
    if (winner >= 0) {
      handleTrigger((uint8_t)g_learn[winner].relay);
      g_block_until_ms = millis() + RF_COOLDOWN_MS;
    }
    aggReset();
  }

  // rc-switch instance and compute-only receive path
  RCSwitch g_rc;

  static bool computeFromRcSwitch(uint32_t &outHash, uint32_t &outSum, uint16_t &outLen) {
    if (!g_rc.available()) return false;
    
    // Read once; rc-switch provides value, bit length, and protocol
    unsigned long value = g_rc.getReceivedValue();
    unsigned int bits = g_rc.getReceivedBitlength();
    unsigned int proto = g_rc.getReceivedProtocol();
    g_rc.resetAvailable();

    if (value == 0 || bits == 0) return false;
    
#ifdef RF_DEBUG
    Serial.printf("[RF] rc-switch recv val=%lu bits=%u proto=%u\n", value, bits, proto);
#endif
    
    // Use (value) as signature; fold protocol into sum/len for coarse and tie-breakers
    outHash = (uint32_t)value;
    // Compose a coarse sum surrogate: bits*8 + proto to space-apart signatures
    outSum = (uint32_t)(bits * 8u + (proto & 0xF));
    outLen = (uint16_t)bits; // treat as length for evFrame detection and learning
    g_last_activity_ms = millis();
    return true;
  }

  // Persistence
  void loadPrefs() {
    g_prefs.begin("tltb_mini", false);
    for (int i = 0; i < 6; ++i) {
      char key[16];
      snprintf(key, sizeof(key), "rf_sig%u", i);
      g_learn[i].sig = g_prefs.getULong(key, 0);
      snprintf(key, sizeof(key), "rf_sum%u", i);
      g_learn[i].sum = g_prefs.getULong(key, 0);
      snprintf(key, sizeof(key), "rf_len%u", i);
      g_learn[i].len = (uint16_t)g_prefs.getUShort(key, 0);
      g_learn[i].relay = (uint8_t)i;
    }
  }

  void saveSlot(int i) {
    char key[16];
    snprintf(key, sizeof(key), "rf_sig%u", i);
    g_prefs.putULong(key, g_learn[i].sig);
    snprintf(key, sizeof(key), "rf_sum%u", i);
    g_prefs.putULong(key, g_learn[i].sum);
    snprintf(key, sizeof(key), "rf_len%u", i);
    g_prefs.putUShort(key, g_learn[i].len);
  }

  // Activate relay with exclusivity
  void handleTrigger(uint8_t rindex) {
    if (rindex >= (uint8_t)R_COUNT) return;
    
    // Simple toggle behavior for all relays
    if (activeRelay == rindex) {
      relayOff((RelayIndex)rindex);
      activeRelay = -1;
      Buzzer::beep();
      return;
    }
    
    // Turn off all other relays and turn on the requested one
    for (int i = 0; i < 6; ++i) relayOff((RelayIndex)i);
    relayOn((RelayIndex)rindex);
    activeRelay = rindex;
    Buzzer::beep();
  }

} // namespace

namespace RF {

bool begin() {
  // use pull-up on data pin
  pinMode(PIN_RF_DATA, INPUT_PULLUP);
  g_rc.enableReceive(digitalPinToInterrupt(PIN_RF_DATA));
  loadPrefs();
  g_last_activity_ms = millis();
  return true;
}

// Runtime: actuate on a single exact match to improve responsiveness.
void service() {
  uint32_t nowMs = millis();
  
  // Handle async learning mode
  if (g_learning.active) {
    if (nowMs >= g_learning.deadline) {
      // Learning timeout
      g_learning.timeout = true;
      g_learning.active = false;
      Serial.printf("[RF] Learning timeout for relay %d\n", g_learning.channel);
      return;
    }
    
    // Try to capture a signal for learning
    uint32_t sig, sum; 
    uint16_t len;
    if (computeFromRcSwitch(sig, sum, len)) {
      // Check for consistency with previous signal
      if (g_learning.lastSig != 0) {
        if (sig == g_learning.lastSig && (nowMs - g_learning.lastAt) <= 2000) {
          // Successful learning
          g_learn[g_learning.channel].sig = sig;
          g_learn[g_learning.channel].sum = sum;
          g_learn[g_learning.channel].len = len;
          g_learn[g_learning.channel].relay = (uint8_t)g_learning.channel;
          saveSlot(g_learning.channel);
          
          Serial.printf("[RF] Learned: sig=0x%08X sum=%u len=%u for relay %d\n", 
                        sig, sum, len, g_learning.channel);
          Buzzer::learnSuccess();
          
          g_learning.success = true;
          g_learning.active = false;
          return;
        }
        // or accept if coarse sum is close on repeat
        uint32_t diff = (g_learning.lastSum > sum) ? (g_learning.lastSum - sum) : (sum - g_learning.lastSum);
        if (diff <= 6 && (nowMs - g_learning.lastAt) <= 2000 && 
            (len + 2 >= g_learning.lastLen && len <= g_learning.lastLen + 2)) {
          // Successful coarse learning
          g_learn[g_learning.channel].sig = sig;
          g_learn[g_learning.channel].sum = sum;
          g_learn[g_learning.channel].len = len;
          g_learn[g_learning.channel].relay = (uint8_t)g_learning.channel;
          saveSlot(g_learning.channel);
          
          Serial.printf("[RF] Learned (coarse): sig=0x%08X sum=%u len=%u for relay %d\n", 
                        sig, sum, len, g_learning.channel);
          Buzzer::learnSuccess();
          
          g_learning.success = true;
          g_learning.active = false;
          return;
        }
      }
      
      // Store this signal for comparison
      g_learning.lastSig = sig;
      g_learning.lastSum = sum;
      g_learning.lastLen = len;
      g_learning.lastAt = nowMs;
    }
    
    // Don't process normal RF commands while learning
    return;
  }
  
  // Normal RF processing (when not learning)
  // If an active burst has gone quiet, finalize it
  if (g_agg.active && ((nowMs - g_agg.lastMs) > BURST_GAP_MS || 
                       (nowMs - g_agg.startMs) > MAX_BURST_MS))
    finalizeBurst();
    
  // If in cooldown, ignore frames
  if (nowMs < g_block_until_ms) return;

  uint32_t sig, sum; 
  uint16_t len;
  // Fetch a frame from rc-switch
  if (!computeFromRcSwitch(sig, sum, len)) return;

  // Determine best candidate index with scoring
  auto scoreOf = [&](int i){
    uint32_t dsum = (g_learn[i].sum > sum) ? (g_learn[i].sum - sum) : (sum - g_learn[i].sum);
    uint16_t glen = g_learn[i].len;
    uint32_t ldiff = (glen > len) ? (glen - len) : (len - glen);
    return (dsum << 4) + ldiff;
  };

  int candidate = -1; 
  uint32_t candScore = 0xFFFFFFFFu;
  
  // Prefer exact signature matches; if multiple, pick closest by score
  for (int i = 0; i < 6; ++i) {
    if (g_learn[i].sig != 0 && g_learn[i].sig == sig) {
      uint32_t sc = scoreOf(i);
      if (sc < candScore) { 
        candScore = sc; 
        candidate = i; 
      }
    }
  }

  // Consider EV-like common bit-lengths as "exact" class for voting weight
  bool evFrame = (len == 24 || len == 28 || len == 32);
  
  if (candidate < 0) {
    // Coarse fallback: require single close match
    int coarseIdx = -1; 
    uint32_t bestSc = 0xFFFFFFFFu; 
    int matches = 0;
    
    for (int i = 0; i < 6; ++i) {
      if (g_learn[i].sig == 0) continue;
      uint32_t dsum = (g_learn[i].sum > sum) ? (g_learn[i].sum - sum) : (sum - g_learn[i].sum);
      uint16_t glen = g_learn[i].len;
      if (dsum <= 6 && len + 2 >= glen && len <= glen + 2) {
        uint32_t sc = scoreOf(i);
        if (sc < bestSc) {
          bestSc = sc; 
          coarseIdx = i; 
        }
        matches++;
      }
    }
    if (matches == 1 && coarseIdx >= 0) { 
      candidate = coarseIdx; 
      candScore = bestSc; 
    }
  }

  // Accumulate vote; do not actuate yet — wait for end of burst for stability
  if (candidate >= 0) {
    if (!g_agg.active) { 
      g_agg.active = true; 
      g_agg.startMs = nowMs; 
    }
    g_agg.lastMs = nowMs;
    
    if (evFrame) {
      g_agg.anyEv = true;
      uint8_t weight = 2; // EV frames are reliable, count more
      uint16_t newVotes = (uint16_t)g_agg.votes[candidate] + weight;
      g_agg.votes[candidate] = (uint8_t)(newVotes > 255 ? 255 : newVotes);
      if (candScore < g_agg.bestScore[candidate]) 
        g_agg.bestScore[candidate] = candScore;
    } else {
      // Only track coarse votes if no EV frames seen in this burst
      if (!g_agg.anyEv) {
        uint16_t newVotes = (uint16_t)g_agg.coarseVotes[candidate] + 1;
        g_agg.coarseVotes[candidate] = (uint8_t)(newVotes > 255 ? 255 : newVotes);
      }
    }
  }
}

bool isPresent() {
  // With a passive OOK receiver and rc-switch, we can't probe hardware presence.
  // Treat RF as present so the system doesn't warn just because no activity occurred recently.
  return true;
}

// Learning: require two consistent captures (within 2s) and decent fingerprint.
bool learn(int relayIndex) {
  if (relayIndex < 0) relayIndex = 0;
  if (relayIndex > 5) relayIndex = 5;
  
  uint32_t deadline = millis() + 30000; // 30 second timeout
  uint32_t lastSig = 0, lastSum = 0, lastAt = 0;
  uint16_t lastLen = 0;

  Serial.printf("[RF] Learning mode for relay %d. Press remote button (30s timeout)...\n", relayIndex);

  while (millis() < deadline) {
    uint32_t sig, sum; 
    uint16_t len;
    if (!computeFromRcSwitch(sig, sum, len)) { 
      delay(6); 
      continue; 
    }
    
    // require basic repeat consistency
    if (lastSig != 0) {
      if (sig == lastSig && (millis() - lastAt) <= 2000) {
        g_learn[relayIndex].sig = sig;
        g_learn[relayIndex].sum = sum;
        g_learn[relayIndex].len = len;
        g_learn[relayIndex].relay = (uint8_t)relayIndex;
        saveSlot(relayIndex);
        Serial.printf("[RF] Learned: sig=0x%08X sum=%u len=%u for relay %d\n", 
                      sig, sum, len, relayIndex);
        Buzzer::learnSuccess();
        return true;
      }
      // or accept if coarse sum is close on repeat
      uint32_t diff = (lastSum > sum) ? (lastSum - sum) : (sum - lastSum);
      if (diff <= 6 && (millis() - lastAt) <= 2000 && 
          (len + 2 >= lastLen && len <= lastLen + 2)) {
        g_learn[relayIndex].sig = sig;
        g_learn[relayIndex].sum = sum;
        g_learn[relayIndex].len = len;
        g_learn[relayIndex].relay = (uint8_t)relayIndex;
        saveSlot(relayIndex);
        Serial.printf("[RF] Learned (coarse): sig=0x%08X sum=%u len=%u for relay %d\n", 
                      sig, sum, len, relayIndex);
        Buzzer::learnSuccess();
        return true;
      }
    }
    lastSig = sig;
    lastSum = sum;
    lastLen = len;
    lastAt = millis();
    delay(6);
  }
  
  Serial.printf("[RF] Learning timeout for relay %d\n", relayIndex);
  return false;
}

bool startLearning(int relayIndex) {
  if (relayIndex < 0 || relayIndex > 5) return false;
  if (g_learning.active) return false; // Already learning
  
  g_learning.active = true;
  g_learning.channel = relayIndex;
  g_learning.startTime = millis();
  g_learning.deadline = g_learning.startTime + 30000; // 30 second timeout
  g_learning.lastSig = 0;
  g_learning.lastSum = 0;
  g_learning.lastLen = 0;
  g_learning.lastAt = 0;
  g_learning.success = false;
  g_learning.timeout = false;
  
  Serial.printf("[RF] Started async learning for relay %d (30s timeout)\n", relayIndex);
  return true;
}

bool isLearning() {
  return g_learning.active;
}

LearningStatus getLearningStatus() {
  LearningStatus status;
  status.active = g_learning.active;
  status.channel = g_learning.channel;
  status.startTime = g_learning.startTime;
  status.success = g_learning.success;
  status.timeout = g_learning.timeout;
  
  if (g_learning.active) {
    uint32_t elapsed = millis() - g_learning.startTime;
    status.timeRemaining = (elapsed < 30000) ? (30000 - elapsed) : 0;
  } else {
    status.timeRemaining = 0;
  }
  
  return status;
}

void stopLearning() {
  if (g_learning.active) {
    Serial.printf("[RF] Stopped learning for relay %d\n", g_learning.channel);
    g_learning.active = false;
  }
}

bool clearAll() {
  // Clear all learned codes
  for (int i = 0; i < 6; ++i) {
    g_learn[i].sig = 0;
    g_learn[i].sum = 0;
    g_learn[i].len = 0;
    g_learn[i].relay = (uint8_t)i;
    saveSlot(i);
  }
  Serial.println("[RF] All learned codes cleared");
  return true;
}

int8_t getActiveRelay() {
  return activeRelay;
}

} // namespace RF