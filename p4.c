#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <pthread.h>
#include "debug.h"
#include "netif.h"
#include "server.h"
#include "client.h"
#include "util.h"

// user provided arguments
int num_nodes;
int gossip_b;
int gossip_c;
int time_to_failure;
int num_failure_nodes;
int time_bw_failures;
int random_seed;

// our globals
nodeInfo *endpoints;
nodeInfo *self;

int self_id;
int last_process = FALSE;
FILE *ep;
pthread_t server_thread;
pthread_barrier_t barrier;
long life_time =0;//It should be num_failuer_nodes * ( time_bw_failures +1)

neighbor_t *neighbors;
int *send_to;

int main(int argc, const char *argv[])
{
  if (argc != 8) {
    // user messed up. Print usage info and exit
    printf("Usage ./p4 N b c F B P S\n \
      number of peer nodes N\n \
      gossip parameter b\n \
      gossip parameter c\n \
      number of seconds after which a node is considered dead F\n \
      number of bad nodes that should fail B\n \
      number of seconds to wait between failures P\n \
      the see of the random number generator S\n");
    exit(1);
  }
  int i=0;
  // take in user's arguments
  num_nodes = atoi(argv[1]);
  gossip_b = atoi(argv[2]);
  gossip_c = atoi(argv[3]);
  time_to_failure = atoi(argv[4]);
  num_failure_nodes = atoi(argv[5]);
  time_bw_failures = atoi(argv[6]);
  random_seed = atoi(argv[7]);

  //storing life of this program
  life_time = num_failure_nodes * ( time_bw_failures +1 );

  //allocate space for neighbors
  neighbors = (neighbor_t *)malloc(num_nodes * sizeof(neighbor_t));
  for(i =0;i<num_nodes;i++){
    neighbors[i].index = -1;
  }

  //allocate space for storing index of neighbors to send heartbeats
  send_to = (int *)malloc(gossip_b * sizeof(int));
  
  // allocate memory for our endpoints
  endpoints = (nodeInfo *)malloc(num_nodes * sizeof(nodeInfo));

  // dispatch server thread
  pthread_barrier_init(&barrier, NULL, NUM_THREADS);
  pthread_create(&server_thread, NULL, &server, NULL);

  // wait
  pthread_barrier_wait(&barrier);
  client();

  free(endpoints);
  return 0;
}


