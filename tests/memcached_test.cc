/* Gearman server and library
 * Copyright (C) 2008 Brian Aker, Eric Day
 * All rights reserved.
 *
 * Use and distribution licensed under the BSD license.  See
 * the COPYING file in the parent directory for full text.
 */

#include <config.h>
#include <libtest/test.hpp>

using namespace libtest;

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include <libgearman/gearman.h>

#include <tests/basic.h>
#include <tests/context.h>

#ifndef __INTEL_COMPILER
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

static in_port_t memcached_port= 0;

static test_return_t gearmand_basic_option_test(void *)
{
  test_true(memcached_port);
  char memcached_server_string[1024];
  int length= snprintf(memcached_server_string, 
                       sizeof(memcached_server_string),
                       "--libmemcached-servers=localhost:%d",
                       int(memcached_port));
  test_true(length > 0);
  const char *args[]= {
    "--check-args",
    "--queue-type=libmemcached",
    memcached_server_string,
    0 };

  test_compare(EXIT_SUCCESS, exec_cmdline(gearmand_binary(), args, true));

  return TEST_SUCCESS;
}

static test_return_t collection_init(void *object)
{
  char memcached_server_string[1024];
  int length= snprintf(memcached_server_string, 
                       sizeof(memcached_server_string),
                       "--libmemcached-servers=localhost:%d",
                       int(memcached_port));
  test_true(length > 0);
  const char *argv[]= {
    memcached_server_string,
    "--queue-type=libmemcached",
    0 };

  Context *test= (Context *)object;
  assert(test);

  test_truth(test->initialize(2, argv));

  return TEST_SUCCESS;
}

static test_return_t collection_cleanup(void *object)
{
  Context *test= (Context *)object;
  test->reset();

  return TEST_SUCCESS;
}


static void *world_create(server_startup_st& servers, test_return_t& error)
{
  if (has_memcached() == false or has_libmemcached() == false)
  {
    error= TEST_SKIPPED;
    return NULL;
  }

  memcached_port= libtest::get_free_port();
  if (server_startup(servers, "memcached", memcached_port, 0, NULL) == false)
  {
    error= TEST_FAILURE;
    return NULL;
  }

  return new Context(default_port(), servers);
}

static bool world_destroy(void *object)
{
  Context *test= (Context *)object;

  delete test;

  return TEST_SUCCESS;
}

test_st gearmand_basic_option_tests[] ={
  {"--queue-type=libmemcached --libmemcached-servers=", 0, gearmand_basic_option_test },
  {0, 0, 0}
};

test_st tests[] ={
  {"gearman_client_echo()", 0, client_echo_test },
  {"gearman_client_echo() fail", 0, client_echo_fail_test },
  {"gearman_worker_echo()", 0, worker_echo_test },
  {"clean", 0, queue_clean },
  {"add", 0, queue_add },
  {"worker", 0, queue_worker },
  {0, 0, 0}
};

collection_st collection[] ={
  {"gearmand options", 0, 0, gearmand_basic_option_tests},
  {"memcached queue", collection_init, collection_cleanup, tests},
  {0, 0, 0, 0}
};

void get_world(Framework *world)
{
  world->collections(collection);
  world->create(world_create);
  world->destroy(world_destroy);
}
