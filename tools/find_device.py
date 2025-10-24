#!/usr/bin/env python3
"""
TLTB Mini Device Discovery Tool
Scans the local network to find TLTB Mini devices

Windows mDNS Note:
Windows doesn't support .local domains by default.
Use this tool to find the device's IP address instead.
"""

import requests
import ipaddress
import threading
from concurrent.futures import ThreadPoolExecutor
import time
import socket

def check_device(ip):
    """Check if IP address has a TLTB Mini device"""
    try:
        # Try the discovery endpoint
        response = requests.get(f"http://{ip}/discover", timeout=2)
        if response.status_code == 200:
            data = response.json()
            if data.get('device') == 'TLTB-Mini':
                return {
                    'ip': ip,
                    'hostname': data.get('hostname', ''),
                    'mac': data.get('mac', ''),
                    'ap_ssid': data.get('ap_ssid', '')
                }
    except:
        pass
    
    # Fallback: try ping endpoint
    try:
        response = requests.get(f"http://{ip}/ping", timeout=1)
        if response.status_code == 200 and response.text == "pong":
            # This might be a TLTB Mini, check the main page
            try:
                response = requests.get(f"http://{ip}/", timeout=2)
                if "TLTB Mini" in response.text:
                    return {'ip': ip, 'hostname': '', 'mac': '', 'ap_ssid': ''}
            except:
                pass
    except:
        pass
    
    return None

def scan_network():
    """Scan the local network for TLTB Mini devices"""
    print("🔍 Scanning for TLTB Mini devices...")
    print("This may take up to 30 seconds...\n")
    
    # Get local IP and scan the subnet
    import socket
    try:
        # Get local IP
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        local_ip = s.getsockname()[0]
        s.close()
        
        # Create network range
        network = ipaddress.IPv4Network(f"{local_ip}/24", strict=False)
        
        print(f"Scanning network: {network}")
        
    except:
        # Fallback to common ranges
        print("Could not detect local network, scanning common ranges...")
        network = ipaddress.IPv4Network("192.168.1.0/24")
    
    devices = []
    
    # Scan with multiple threads for speed
    with ThreadPoolExecutor(max_workers=50) as executor:
        futures = [executor.submit(check_device, str(ip)) for ip in network.hosts()]
        
        for future in futures:
            result = future.result()
            if result:
                devices.append(result)
    
    return devices

def main():
    print("=" * 50)
    print("🚛 TLTB Mini Device Discovery")
    print("=" * 50)
    
    devices = scan_network()
    
    if devices:
        print(f"\n✅ Found {len(devices)} TLTB Mini device(s):\n")
        
        for i, device in enumerate(devices, 1):
            print(f"{i}. Device at {device['ip']}")
            if device['hostname']:
                print(f"   Hostname: {device['hostname']}")
            if device['mac']:
                print(f"   MAC: {device['mac']}")
            if device['ap_ssid']:
                print(f"   AP SSID: {device['ap_ssid']}")
            
            print(f"   🌐 Web Interface: http://{device['ip']}")
            if device['hostname']:
                print(f"   🌐 Local Address: http://{device['hostname']}")
            print()
        
        print("💡 Bookmark these addresses for easy future access!")
        
    else:
        print("\n❌ No TLTB Mini devices found on the network.")
        print("\nTroubleshooting:")
        print("1. Make sure the TLTB Mini is powered on")
        print("2. Check that it's connected to the same WiFi network")
        print("3. If in AP mode, connect to TLTB-Mini-XXXXXX and go to http://192.168.4.1")
        print("4. Windows Note: .local domains don't work by default on Windows")
        print("   Install iTunes or Bonjour Print Services for .local domain support")

def test_mdns():
    """Test if mDNS resolution works on this system"""
    print("\n🔍 Testing mDNS resolution...")
    try:
        ip = socket.gethostbyname('tltb-mini.local')
        print(f"✅ mDNS works! tltb-mini.local resolves to {ip}")
        return ip
    except socket.gaierror:
        print("❌ mDNS not supported on this system")
        print("   This is normal for Windows without Bonjour services")
        return None

if __name__ == "__main__":
    # Test mDNS first
    mdns_ip = test_mdns()
    
    # Run the main discovery
    main()