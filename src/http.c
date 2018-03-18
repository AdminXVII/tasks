#include "backend.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <microhttpd.h>

#define BUF_SIZE (MAX_MSG_LEN+27)

message_t *head;
struct MHD_Response* not_allowed; // static response to all request except GET

void sendUpdate(byte uid, char *msg, message_type type){
    /*conn_t *conn, *prev;
    
    if (type == END){
        sprintf(buf, "event: end\ndata: %d\n\n", uid);
    } else {
        sprintf(buf, "event: msg\ndata: {%d,\"%s\"}\n\n", uid, msg);
    }
    conn = conns;
    prev = NULL;
    while(conn->next != NULL){
        if(buf_write(conn->fd, buf, strlen(buf)) == -1){
            if(prev != NULL)
                prev->next = conn->next; // because only a swap is performed, no locking is needed
            close(conn->fd);
            free(conn);
        }
        prev = conn;
        conn = conn->next;
    }
    // if(type==NAME)
    // store for later use
    */
}

int spawn_server(int fd, int port) {
  // Epoll is vastly superior but only supported in Linux
  enum MHD_FLAG dispatch = (MHD_is_feature_supported(MHD_FEATURE_EPOLL) ? MHD_USE_EPOLL : MHD_USE_POLL);
  struct MHD_Daemon* daemon;
  
  if (fd != -1) { // we have a valid file descriptor
    daemon = MHD_start_daemon(
      dispatch | MHD_USE_DUAL_STACK | MHD_USE_SELECT_INTERNALLY, port,
      NULL, NULL, // accept everyone
      &init_connection, NULL,
      MHD_OPTION_LISTEN_SOCKET, fd,
      MHD_OPTION_THREAD_POOL_SIZE, 10,
      MHD_OPTION_END);
  } else {
    daemon = MHD_start_daemon(
      dispatch | MHD_USE_DUAL_STACK | MHD_USE_SELECT_INTERNALLY, port,
      NULL, NULL, // accept all IPs
      &init_connection, NULL,
      MHD_OPTION_THREAD_POOL_SIZE, 10,
      MHD_OPTION_END);
  }
  
  not_allowed = MHD_create_response_from_buffer(0, "", MHD_RESPMEM_PERSISTENT); // change when real buffer
  if (!not_allowed)
      return MHD_NO;
  MHD_add_response_header(not_allowed, "Accept-Ranges", "none");
  MHD_add_response_header(not_allowed, "Access-Control-Allow-Origin", "*");
  
  head = (message_t *)malloc(sizeof(message_t));
  head->msg = "It works";
  head->len = strlen(head->msg);
  head->references = 0;
  head->next = head;
      
  return (daemon != NULL);
}

int init_connection(void* cls, struct MHD_Connection* connection,
                         const char* url, const char* method,
                         const char* version, const char* upload_data,
                         size_t* upload_data_size, void** con_cls) {
    if (NULL == *con_cls) {
      *con_cls = (void*)true;
      return MHD_YES;
    }

    if (strcmp(method, "GET") == 0) {
      struct MHD_Response* response = MHD_create_response_from_callback(-1, BUF_SIZE, &get_data, (void *)new_stream(), &cancel_client);
      if (!response)
          return MHD_NO;
      MHD_add_response_header(response, "Accept-Ranges", "none");
      MHD_add_response_header(response, "Cache-Control", "no-cache");
      MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
      MHD_add_response_header(response, "Content-type", "text/event-stream");
      printf("Created headers\n");
      
      return MHD_queue_response(connection, MHD_HTTP_OK, response);
    } else
      return MHD_queue_response(connection, 405, not_allowed);
}

int get_data (void *cls, uint64_t position, char *buf, size_t max){
  position_t *pos = (position_t *)cls;
  message_t *current = pos->message;
  int offset = pos->offset;
  int tot_len = 0;
  int copy_len;
  
  while (max > 0 && current != NULL){
    if (offset != 0) {
      copy_len = (max > (current->len - offset))? (current->len - offset) : max; // copy at most copy_len bytes
      memcpy(buf, current->msg + offset, copy_len);
      copy_len = 0;
    } else {
      copy_len = (max > current->len)? current->len : max; // copy at most copy_len bytes
      memcpy(buf, current->msg, copy_len);
    }
    max -= copy_len;
    tot_len += copy_len;
    
    message_t *next = current->next;
    __atomic_add_fetch(&(next->references), 1, __ATOMIC_SEQ_CST); // Ensure next node doesn't get deleted
    if (__atomic_sub_fetch(&(current->references), 1, __ATOMIC_SEQ_CST) == 0) { // Then delete this node if no one will use it
      free(current);
    }
    
    current = next;
    offset = 0;
  }
  
  pos->message = current;
  pos->offset = current->len - copy_len;
  return tot_len;
}

void cancel_client (void *cls){
  message_t *current = ((position_t *)cls)->message;
  if (__atomic_sub_fetch(&(current->references), 1, __ATOMIC_SEQ_CST) == 0) {
    free(current);
  }
  free(cls);
  printf("Client freed\n");
}

position_t *new_stream(){
  position_t *stream = (position_t *)malloc(sizeof(position_t));
  stream->offset = 0;
  stream->message = head;
  
  return stream;
}

