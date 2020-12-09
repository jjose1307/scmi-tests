/** @file
 * Copyright (c) 2020, Arm Limited or its affiliates. All rights reserved.
 * SPDX-License-Identifier : Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
**/

#include "val_interface.h"
#include "val_performance.h"

#define TEST_NUM  (SCMI_PERFORMANCE_TEST_NUM_BASE + 21)
#define TEST_DESC "Performance describe fast channel check      "

#define PARAMETER_SIZE 2

uint32_t performance_query_describe_fast_channel(void)
{
    int32_t  status;
    uint32_t rsp_msg_hdr;
    uint32_t cmd_msg_hdr;
    size_t   param_count;
    size_t   return_value_count;
    uint32_t return_values[MAX_RETURNS_SIZE];
    uint32_t domain_id, num_domains, message_id, attributes;
    uint32_t parameters[PARAMETER_SIZE];
    uint32_t run_flag = 0;

    if (val_test_initialize(TEST_NUM, TEST_DESC) != VAL_STATUS_PASS)
        return VAL_STATUS_SKIP;

    /* Skip if describe fast channel is not supported*/
    val_print(VAL_PRINT_TEST, "\n     [Check 1] Query performance describe fast channel support");

    VAL_INIT_TEST_PARAM(param_count, rsp_msg_hdr, return_value_count, status);
    message_id = PERFORMANCE_DESCRIBE_FASTCHANNEL;
    param_count++;
    cmd_msg_hdr = val_msg_hdr_create(PROTOCOL_PERFORMANCE, PERFORMANCE_PROTOCOL_MESSAGE_ATTRIBUTES,
                                     COMMAND_MSG);
    val_send_message(cmd_msg_hdr, param_count, &message_id, &rsp_msg_hdr, &status,
                     &return_value_count, return_values);

    if (status == SCMI_NOT_FOUND) {
        val_print(VAL_PRINT_ERR, "\n       PERF_DESCRIBE_FASTCHANNEL not supported     ");
        return VAL_STATUS_SKIP;
    }

    /* Check performance describe fast channel for valid domain */
    num_domains = val_performance_get_info(NUM_PERF_DOMAINS, 0x00);
    if (num_domains == 0) {
        val_print(VAL_PRINT_ERR, "\n       No performance domains found                ");
        return VAL_STATUS_SKIP;
    }
    val_print(VAL_PRINT_DEBUG, "\n       NUM DOMAINS    : %d", num_domains);

    for (domain_id = 0; domain_id < num_domains; domain_id++)
    {
        if (val_performance_get_info(PERF_DOMAIN_FAST_CH_SUPPORT, domain_id) == 0)
            continue;

        for (message_id = PERFORMANCE_LIMITS_SET; message_id <= PERFORMANCE_LEVEL_GET; message_id++)
        {
            if (val_performance_get_info(PERF_MESSAGE_FAST_CH_SUPPORT, message_id) == 0)
                continue;
            run_flag = 1;

            val_print(VAL_PRINT_TEST, "\n     PERFORMANCE DOMAIN ID:%d and msg_id: %d",
                                       domain_id, message_id);
            val_print(VAL_PRINT_TEST, "\n     [Check 2] Query describe fast channel");


            VAL_INIT_TEST_PARAM(param_count, rsp_msg_hdr, return_value_count, status);
            parameters[param_count++] = domain_id;
            parameters[param_count++] = message_id;

            cmd_msg_hdr = val_msg_hdr_create(PROTOCOL_PERFORMANCE, PERFORMANCE_DESCRIBE_FASTCHANNEL,
                                             COMMAND_MSG);
            val_send_message(cmd_msg_hdr, param_count, parameters, &rsp_msg_hdr, &status,
                             &return_value_count, return_values);

            if (val_compare_status(status, SCMI_SUCCESS) != VAL_STATUS_PASS)
                return VAL_STATUS_FAIL;

            if (val_compare_msg_hdr(cmd_msg_hdr, rsp_msg_hdr) != VAL_STATUS_PASS)
                return VAL_STATUS_FAIL;

            val_print_return_values(return_value_count, return_values);

            attributes = return_values[ATTRIBUTE_OFFSET];
            if (val_reserved_bits_check_is_zero(VAL_EXTRACT_BITS(attributes, 3, 31)) !=
                                                VAL_STATUS_PASS)
                return VAL_STATUS_FAIL;

            val_print(VAL_PRINT_TEST, "\n       DOORBELL SUPPORT       :%d",
                                       VAL_EXTRACT_BITS(attributes, 0, 0));
            val_print(VAL_PRINT_TEST, "\n       DOORBELL DATA WIDTH    :%d",
                                       VAL_EXTRACT_BITS(attributes, 1, 2));
            val_print(VAL_PRINT_TEST, "\n       FAST CH RATE LIMIT     :%d",
                                       return_values[FAST_CH_RATE_LIMIT_OFFSET]);
            val_print(VAL_PRINT_TEST, "\n       FAST CH ADDR LOW       :%d",
                                       return_values[CHAN_ADDR_LOW_OFFSET]);
            val_print(VAL_PRINT_TEST, "\n       FAST CH ADDR HIGH      :%d",
                                       return_values[CHAN_ADDR_HIGH_OFFSET]);
            val_print(VAL_PRINT_TEST, "\n       FAST CH SIZE           :%d",
                                       return_values[CHAN_SIZE_OFFSET]);
            val_print(VAL_PRINT_TEST, "\n       DOORBELL ADDR LOW      :%d",
                                       return_values[DOORBELL_ADDR_LOW_OFFSET]);
            val_print(VAL_PRINT_TEST, "\n       DOORBELL ADDR HIGH     :%d",
                                       return_values[DOORBELL_ADDR_HIGH_OFFSET]);
            val_print(VAL_PRINT_TEST, "\n       DOORBELL MASK LOW      :%d",
                                       return_values[DOORBELL_MASK_LOW_OFFSET]);
            val_print(VAL_PRINT_TEST, "\n       DOORBELL MASK HIGH     :%d",
                                       return_values[DOORBELL_MASK_HIGH_OFFSET]);
            val_print(VAL_PRINT_TEST, "\n       DOORBELL PRESERVE LOW  :%d",
                                       return_values[DOORBELL_PRESERVE_LOW_OFFSET]);
            val_print(VAL_PRINT_TEST, "\n       DOORBELL PRESERVE HIGH :%d",
                                       return_values[DOORBELL_PRESERVE_HIGH_OFFSET]);
        }
    }
    if (run_flag == 0) {
        val_print(VAL_PRINT_ERR, "\n       No domain support FAST CHANNEL              ");
        return VAL_STATUS_SKIP;
    }
    return VAL_STATUS_PASS;
}
