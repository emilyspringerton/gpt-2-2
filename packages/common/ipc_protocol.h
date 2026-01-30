#ifndef IPC_PROTOCOL_H
#define IPC_PROTOCOL_H

#include <stdint.h>

#define SHANKPIT_IPC_SOCKET "/tmp/shankpit_miner.sock"
#define MAGIC_HEADER 0x534B4154 // "SKAT"
#define HMAC_SIZE 32
#define CHALLENGE_SIZE 32

// Message Types
enum MsgType {
    MSG_HELLO       = 0x01,
    MSG_CHALLENGE   = 0x02,
    MSG_ATTEST      = 0x03,
    MSG_HEARTBEAT   = 0x04, // Batched by default now
    MSG_MINING_CTRL = 0x05,
    MSG_GAME_EVENT  = 0x06, // ProofOfFrag
    MSG_ERROR       = 0xFF
};

// Generic Header
typedef struct {
    uint32_t magic;
    uint8_t  type;
    uint32_t payload_len;
} __attribute__((packed)) IPCHeader;

// 3.2.4 Handshake: Hello (Client -> Node)
typedef struct {
    uint32_t game_pid;
    uint32_t build_hash;
    uint64_t session_nonce; // Random start nonce
    uint64_t timestamp;
} __attribute__((packed)) MsgHello;

// 3.2.4 Handshake: Challenge (Node -> Client)
typedef struct {
    uint8_t  challenge_nonce[CHALLENGE_SIZE];
    uint32_t required_entropy_window;
    uint8_t  min_fps;
} __attribute__((packed)) MsgChallenge;

// 3.2.4 Handshake: Attest (Client -> Node)
typedef struct {
    uint64_t session_nonce;
    uint8_t  response_hmac[HMAC_SIZE]; // HMAC(challenge, build_secret)
} __attribute__((packed)) MsgAttest;

// 3.2.5 Activity Heartbeat (Batched - 4Hz)
typedef struct {
    uint64_t seq_id;          // Monotonic
    uint8_t  sample_count;    // How many frames aggregated
    float    frame_time_avg;
    float    frame_time_var;  // Variance (detects V-Sync/Limiters)
    uint8_t  fps_min;
    float    input_entropy;   // Calculated Score
    uint64_t input_hash;      // H(raw_inputs)
    uint64_t timestamp;
    uint8_t  hmac[HMAC_SIZE]; // HMAC(session_key, content)
} __attribute__((packed)) MsgBatchedHeartbeat;

// 3.2.6 Proof of Frag (Server Signed Event)
typedef struct {
    uint64_t match_id;
    uint64_t kill_timestamp;
    uint8_t  victim_id;
    uint8_t  weapon_id;
    uint8_t  server_signature[64]; // Ed25519 signature from Game Server
    uint8_t  hmac[HMAC_SIZE];      // IPC Session HMAC
} __attribute__((packed)) MsgProofOfFrag;

// 3.2.7 Mining Control (Node -> Client)
typedef struct {
    float    target_intensity; // 0.0 - 1.0
    uint64_t hashrate;
    uint64_t pending_rewards;
    uint64_t blocks_mined;
} __attribute__((packed)) MsgMiningStatus;

#endif
