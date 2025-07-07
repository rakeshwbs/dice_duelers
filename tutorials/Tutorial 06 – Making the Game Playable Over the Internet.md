# Tutorial 6 – Making Your Game Playable Over the Internet

### What It Will Cover:

- Why local IPs don’t work over the Internet
- Public IP vs NAT vs private IP addresses
- How to set up port forwarding on a router
- Using Dynamic DNS to handle changing public IPs
- Firewall exceptions (Windows + router)
- Security risks of exposing a port publicly
- Suggestions for future-proofing: TLS, matchmaking servers, or tunneling

> This tutorial will be **platform-specific (Windows)**, practical, and grounded in your exact use case.

**Objective:**
 Transform your local LAN game into a working **online multiplayer experience**, enabling players on **different networks** to connect and play seamlessly.

## Why LAN Isn’t Enough

In your game:

- The **host** listens on a port like `5000`
- The **client** connects to an IP like `192.168.1.10`

✅ This works perfectly **inside your local network** (Wi-Fi or Ethernet)

❌ But it fails **over the Internet** — why?

Because `192.168.x.x`, `10.x.x.x`, and `172.16.x.x` are **private IPs**. They’re not reachable from outside your network.

### Making the Game Work Over the Internet

#### 1️⃣ Host Needs a **Public IP Address**

Go to https://whatismyip.com on the host machine.

You’ll see something like:

> Your Public IP: 102.158.45.77

This is the IP that the **client must use** to connect from another network.

#### 2️⃣ Set Up **Port Forwarding** on the Host’s Router

Most routers block inbound connections by default. You must **forward the game port (e.g., 5000)** to your host PC.

#####  Steps:

1. Login to your router (usually at `192.168.1.1`)
2. Find **Port Forwarding / Virtual Server** section
3. Add a rule:
   - **Port**: 5000
   - **Protocol**: TCP
   - **Target IP**: Host’s local IP (e.g., `192.168.1.10`)
4. Save and reboot the router (if needed)

#### 3️⃣ Allow Inbound Traffic on Windows Firewall

##### On the host:

1. ##### Open **Windows Defender Firewall**

2. Go to **Advanced Settings**

3. Add **Inbound Rule**

   - Port: `5000`
   - Protocol: `TCP`
   - Action: Allow
   - Profile: All (Private/Public)

Without this, the OS might still silently block the game.

#### 4️⃣ Use **Dynamic DNS** (Optional but recommended)

Most ISPs give dynamic IPs (they change every few days). To avoid needing to re-check the IP each time, use a **Dynamic DNS (DDNS)** service.

####  Tools:

- [noip.com](https://www.noip.com/)
- [duckdns.org](https://www.duckdns.org/)

You’ll get a domain like:

> dicehost.ddns.net

Clients can now connect using this domain instead of the IP.

#### 5️⃣ Modify Client Code to Use Public IP or Domain

In `client_main.cpp`:

```c++
std::string serverIP;
std::cout << "Enter server IP or domain: ";
std::getline(std::cin, serverIP);
net.connectToHost(serverIP, 5000);
```

### Security Considerations

When exposing your computer to the Internet, be cautious:

| Risk                    | Mitigation                                     |
| ----------------------- | ---------------------------------------------- |
| **Port Scanning**       | Don’t forward unnecessary ports                |
| **Buffer Overflow**     | Always sanitize and validate input             |
| **DDoS or Abuse**       | Not likely in personal games, but possible     |
| **Message Tampering**   | Consider using encryption (future enhancement) |
| **Unauthorized Access** | Require name verification or simple auth       |

### Advanced Alternatives (Future Ideas)

| Feature               | Description                                         |
| --------------------- | --------------------------------------------------- |
| 🛡 VPN                 | Use Hamachi or ZeroTier to simulate LAN             |
| 🧠 Matchmaking Server  | Central server coordinates client-host pairing      |
| 📦 P2P or Relay Server | Middle server forwards data if NAT blocks direct IP |
| 🔒 TLS encryption      | Use OpenSSL to encrypt messages                     |