/*
 * Copyright 2014-2023 Jetperch LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file
 *
 * @brief Joulescope driver C template example program.
 *
 * Customize this program!
 *
 * References:
 * - https://joulescope-driver.readthedocs.io/en/latest/
 * - https://github.com/jetperch/joulescope_driver
 * - https://github.com/jetperch/joulescope_driver/blob/main/test/jsdrv_util.c
 * - https://github.com/jetperch/joulescope_driver/tree/main/test/jsdrv_util
 *
 * To discover the topics that your device supports, use python:
 *
 * python -m pip install -U pyjoulescope_driver
 * python -m pyjoulescope_driver info *
 */

#include "jsdrv.h"
#include "jsdrv/cstr.h"   // make things easier for us!
#include "jsdrv/topic.h"  // make things easier for us!
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#define ERROR_PREFIX "## ERROR: "
#define MAX_DEVICES_LENGTH (4096U)

// Macro to return from the current function on error
#define ROE(x) do {                                                 \
    int32_t rc__ = (x);                                             \
    if (0 != rc__) {                                                \
        printf(ERROR_PREFIX "%s returned %d\n", #x, (int) rc__);    \
        return (int) rc__;                                          \
    }                                                               \
} while (0)

#define ARG_CONSUME() --argc; ++argv
#define ARG_REQUIRE()  if (argc <= 0) {return usage();}

static volatile int quit_ = 0;

// cross-platform handler for CTRL-C to exit program
static void signal_handler(int signal){
    if ((signal == SIGABRT) || (signal == SIGINT)) {
        quit_ = 1;
    }
}

// convenience wrapper for jsdrv_publish
static int32_t publish(struct jsdrv_context_s * context, const char * device, const char * topic, const struct jsdrv_union_s * value) {
    struct jsdrv_topic_s t;
    jsdrv_topic_set(&t, device);
    jsdrv_topic_append(&t, topic);
    return jsdrv_publish(context, t.topic, value, JSDRV_TIMEOUT_MS_DEFAULT);
}

// convenience wrapper for jsdrv_subscribe
static int32_t subscribe(struct jsdrv_context_s * context, const char * device,
                         const char * topic, uint8_t flags,
                         jsdrv_subscribe_fn cbk_fn, void * cbk_user_data) {
    struct jsdrv_topic_s t;
    jsdrv_topic_set(&t, device);
    jsdrv_topic_append(&t, topic);
    return jsdrv_subscribe(context, t.topic, flags, cbk_fn, cbk_user_data, JSDRV_TIMEOUT_MS_DEFAULT);
}

// Callback for statistics data
static void on_statistics_value(void * user_data, const char * topic, const struct jsdrv_union_s * value) {
    (void) user_data;
    (void) topic;
    struct jsdrv_statistics_s * s = (struct jsdrv_statistics_s *) value->value.bin;
    printf("%" PRId64   ",%g,%g,%g,%g,"   "%g,%g,%g,%g,"   "%g,%g,%g,%g,"  "%g,%g\n",
            s->block_sample_id,
            s->i_avg, s->i_std, s->i_min, s->i_max,
            s->v_avg, s->v_std, s->v_min, s->v_max,
            s->p_avg, s->p_std, s->p_min, s->p_max,
            s->charge_f64, s->energy_f64);
}

static void sleep_ms(uint32_t milliseconds) {
#ifdef WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}

static int usage(void) {
    printf("usage: <exe> [--scnt <value]\n"
           "\n"
           "Options:\n"
           "    --scnt   The number of 1 Msps samples per entry.\n"
           "             Range 1000 to 1000000.  Default is 500000.\n"
    );
    return 1;
}

int main(int argc, char * argv[]) {
    (void) argc;
    (void) argv;
    uint32_t scnt = 500000;

    // argv[0] is the executable path.
    ARG_REQUIRE();
    ARG_CONSUME();

    while (argc) {
        if (argv[0][0] != '-') {
            return usage();
        } else if (0 == strcmp(argv[0], "--scnt")) {
            ARG_CONSUME();
            ARG_REQUIRE();
            ROE(jsdrv_cstr_to_u32(argv[0], &scnt));
            ARG_CONSUME();
        } else {
            return usage();
        }
    }

    struct jsdrv_context_s * context;
    char devices_str[MAX_DEVICES_LENGTH] = "";

    struct jsdrv_union_s devices_value = jsdrv_union_str(devices_str);
    devices_value.size = sizeof(devices_str);

    ROE(jsdrv_initialize(&context, NULL, 1000));
    ROE(jsdrv_query(context, JSDRV_MSG_DEVICE_LIST, &devices_value, 0));
    if (0 == devices_str[0]) {
        printf(ERROR_PREFIX "no Joulescope device found\n");
        return 1;
    }
    if (0 != strchr(devices_str, ',')) {
        printf(ERROR_PREFIX "more than one Joulescope device found\n");
        return 1;
    }
    char * device = devices_str;

    if (!jsdrv_cstr_starts_with(device, "u/js220")) {
        printf(ERROR_PREFIX "this example only supports the JS220, found %s\n", device);
        return 1;
    }
    printf("# Found device %s\n", device);
    ROE(jsdrv_open(context, device, JSDRV_DEVICE_OPEN_MODE_DEFAULTS));

    // Set the current range (optional)
    ROE(publish(context, device, "s/i/range/mode", &jsdrv_union_cstr_r("auto")));

    // Set the voltage range (optional)
    ROE(publish(context, device, "s/v/range/mode", &jsdrv_union_cstr_r("manual")));
    ROE(publish(context, device, "s/v/range/select", &jsdrv_union_cstr_r("15 V")));

    // configure the samples per statistics update in 1 Msps samples
    ROE(publish(context, device, "s/stats/scnt", &jsdrv_union_u8_r(scnt)));

    // subscribe to get statistics updates
    ROE(subscribe(context, device, "s/stats/value", JSDRV_SFLAG_PUB, on_statistics_value, NULL));

    // display the column header
    printf("# sampled_id,"
           "i_avg,i_std,i_min,i_max,"
           "v_avg,v_std,v_min,v_max,"
           "p_avg,p_std,p_min,p_max,"
           "charge,energy\n");

    // Start the statistics streaming
    ROE(publish(context, device, "s/stats/ctrl", &jsdrv_union_u8_r(1)));

    // process incoming data
    signal(SIGABRT, signal_handler);
    signal(SIGINT, signal_handler);
    while (!quit_) {
        sleep_ms(10);
    }

    jsdrv_finalize(context, 1000);
    printf("# SUCCESS\n");
    return 0;
}
