#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/coap.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/tls_credentials.h>

#include "logic/logic.h"

LOG_MODULE_REGISTER(coap_server, LOG_LEVEL_INF);

#define CONFIG_MAX_COAP_MSG_LEN 256

#define PSK_TAG 1

static int server_sock;

static int send_state_response(struct coap_packet *request,
                               struct sockaddr *addr, socklen_t addr_len,
                               state_msg_t *state, uint8_t response_code) {
  struct coap_packet response;
  uint8_t response_buf[128];
  uint8_t token[COAP_TOKEN_MAX_LEN];
  uint8_t token_len = coap_header_get_token(request, token);
  uint16_t id = coap_header_get_id(request);

  int res = coap_packet_init(&response, response_buf, sizeof(response_buf),
                             COAP_VERSION_1, COAP_TYPE_ACK, token_len, token,
                             response_code, id);
  if (res < 0)
    return res;

  // Append the binary struct to the payload
  coap_packet_append_payload_marker(&response);
  coap_packet_append_payload(&response, (uint8_t *)state, sizeof(state_msg_t));

  zsock_sendto(server_sock, response.data, response.offset, 0, addr, addr_len);
  return 0;
}

static int state_get_handler(struct coap_resource *resource, struct coap_packet *request,
                             struct sockaddr *addr, socklen_t addr_len) {
    state_msg_t current_state;
    get_curr_state(&current_state);
    return send_state_response(request, addr, addr_len, &current_state, COAP_RESPONSE_CODE_CONTENT);
}

static int reset_get_handler(struct coap_resource *resource, struct coap_packet *request,
                             struct sockaddr *addr, socklen_t addr_len) {
    state_msg_t reset_st;
    reset_state(&reset_st);
    return send_state_response(request, addr, addr_len, &reset_st, COAP_RESPONSE_CODE_CONTENT);
}

static int entry_post_handler(struct coap_resource *resource, struct coap_packet *request,
                              struct sockaddr *addr, socklen_t addr_len) {
    uint16_t payload_len;
    const uint8_t *payload = coap_packet_get_payload(request, &payload_len);

    if (payload && payload_len == sizeof(entry_msg_t)) {
        entry_msg_t new_entry;
        memcpy(&new_entry, payload, sizeof(entry_msg_t));
        
        // Ensure name is null terminated if it's exactly 8 chars
        new_entry.name[7] = '\0'; 

        state_msg_t updated_state;
        recv_entry(&new_entry, &updated_state);
        
        return send_state_response(request, addr, addr_len, &updated_state, COAP_RESPONSE_CODE_CHANGED);
    }
    
    LOG_ERR("Invalid entry payload size: Expected %d, got %d", sizeof(entry_msg_t), payload_len);
    return -EINVAL; // Or send a CoAP 4.00 Bad Request
}

static const char * const state_path[] = { "state", NULL };
static const char * const reset_path[] = { "reset", NULL };
static const char * const entry_path[] = { "entry", NULL };

static struct coap_resource resources[] = {
    { .path = state_path, .get = state_get_handler },
    { .path = reset_path, .get = reset_get_handler },
    { .path = entry_path, .post = entry_post_handler },
    { NULL }
};

static void coap_serv_thread(void) 
{
    /* 1. Register PSK Credentials */
    tls_credential_add(PSK_TAG, TLS_CREDENTIAL_PSK_ID, CONFIG_APP_PSK_ID, strlen(CONFIG_APP_PSK_ID));
    tls_credential_add(PSK_TAG, TLS_CREDENTIAL_PSK, CONFIG_APP_PSK, strlen(CONFIG_APP_PSK));

    struct sockaddr_in bind_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(CONFIG_APP_COAPS_PORT),
        .sin_addr.s_addr = htonl(INADDR_ANY)
    };

    /* 2. Create DTLS Socket */
    server_sock = zsock_socket(AF_INET, SOCK_DGRAM, IPPROTO_DTLS_1_2);
    if (server_sock < 0) {
        LOG_ERR("Failed to create DTLS socket (err: %d)", errno);
        return;
    }

    /* 3. Attach the PSK Tag and set to SERVER mode */
    sec_tag_t sec_tag_list[] = { PSK_TAG };
    if (zsock_setsockopt(server_sock, SOL_TLS, TLS_SEC_TAG_LIST, sec_tag_list, sizeof(sec_tag_list)) < 0) {
        LOG_ERR("Failed to attach PSK tag (err: %d)", errno);
    }
    
    int role = TLS_DTLS_ROLE_SERVER;
    if (zsock_setsockopt(server_sock, SOL_TLS, TLS_DTLS_ROLE, &role, sizeof(role)) < 0) {
        LOG_ERR("Failed to set DTLS role (err: %d)", errno);
    }

    if (zsock_bind(server_sock, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) < 0) {
        LOG_ERR("Failed to bind DTLS socket (err: %d)", errno);
        return;
    }

    LOG_INF("Secure CoAPS Server started on port %d", CONFIG_APP_COAPS_PORT);

    uint8_t rx_buf[CONFIG_MAX_COAP_MSG_LEN];
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    while (1) {
        int recv_len = zsock_recvfrom(server_sock, rx_buf, sizeof(rx_buf), 0,
                                      (struct sockaddr *)&client_addr, &client_addr_len);
        if (recv_len > 0) {
            struct coap_packet request;
            struct coap_option options[16];
            
            if (coap_packet_parse(&request, rx_buf, recv_len, options, 16) == 0) {
                coap_handle_request(&request, resources, options, 16,
                                    (struct sockaddr *)&client_addr, client_addr_len);
            }
        }
    }
}

K_THREAD_DEFINE(coap_tid, 4096, coap_serv_thread, NULL, NULL, NULL, 7, 0, 0);