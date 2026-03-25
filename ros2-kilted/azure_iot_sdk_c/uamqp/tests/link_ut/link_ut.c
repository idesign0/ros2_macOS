// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#else
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#endif

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umock_c_negative_tests.h"
#include "umock_c/umocktypes_bool.h"
#include "umock_c/umocktypes_stdint.h"

static void* my_gballoc_malloc(size_t size)
{
    return malloc(size);
}

static void* my_gballoc_calloc(size_t nmemb, size_t size)
{
    return calloc(nmemb, size);
}

static void* my_gballoc_realloc(void* ptr, size_t size)
{
    return realloc(ptr, size);
}

static void my_gballoc_free(void* ptr)
{
    free(ptr);
}

#define ENABLE_MOCKS

#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/singlylinkedlist.h"
#include "azure_c_shared_utility/tickcounter.h"
#include "azure_uamqp_c/session.h"
#include "azure_uamqp_c/amqpvalue.h"
#include "azure_uamqp_c/amqp_definitions.h"
#include "azure_uamqp_c/amqp_frame_codec.h"
#include "azure_uamqp_c/async_operation.h"

#undef ENABLE_MOCK_FILTERING_SWITCH
#define ENABLE_MOCK_FILTERING
#define please_mock_transfer_set_delivery_tag MOCK_ENABLED
#undef ENABLE_MOCK_FILTERING_SWITCH
#undef ENABLE_MOCK_FILTERING

#undef ENABLE_MOCKS

#include "azure_uamqp_c/link.h"

static SESSION_HANDLE TEST_SESSION_HANDLE = (SESSION_HANDLE)0x4000;
const char* TEST_LINK_NAME_1 = "test_link_name_1";
static TICK_COUNTER_HANDLE TEST_TICK_COUNTER_HANDLE = (TICK_COUNTER_HANDLE)0x4001;
static SINGLYLINKEDLIST_HANDLE TEST_SINGLYLINKEDLIST_HANDLE = (SINGLYLINKEDLIST_HANDLE)0x4002;
static LINK_ENDPOINT_HANDLE TEST_LINK_ENDPOINT = (LINK_ENDPOINT_HANDLE)0x4003;
const AMQP_VALUE TEST_LINK_SOURCE = (AMQP_VALUE)0x4004;
const AMQP_VALUE TEST_LINK_TARGET = (AMQP_VALUE)0x4005;
const AMQP_VALUE TEST_AMQP_VALUE = (AMQP_VALUE)0x4006;
const LIST_ITEM_HANDLE TEST_LIST_ITEM_HANDLE = (LIST_ITEM_HANDLE)0x4007;
const TRANSFER_HANDLE TEST_TRANSFER_HANDLE = (TRANSFER_HANDLE)0x4008;
#define AMQP_BATCHING_FORMAT_CODE 0x80013700

static TEST_MUTEX_HANDLE g_testByTest;

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

static int umocktypes_copy_bool_ptr(bool** destination, const bool* source)
{
    int result;

    *destination = (bool*)my_gballoc_malloc(sizeof(bool));
    if (*destination == NULL)
    {
        result = MU_FAILURE;
    }
    else
    {
        *(*destination) = (*source);

        result = 0;
    }

    return result;
}

static void umocktypes_free_bool_ptr(bool** value)
{
    if (*value != NULL)
    {
        my_gballoc_free(*value);
    }
}

static char* umocktypes_stringify_bool_ptr(const bool** value)
{
    char* result;

    result = (char*)my_gballoc_malloc(8);
    if (result != NULL)
    {
        if (*value == NULL)
        {
            (void)strcpy(result, "{NULL}");
        }
        else if (*(*value) == true)
        {
            (void)strcpy(result, "{true}");
        }
        else
        {
            (void)strcpy(result, "{false}");
        }
    }

    return result;
}

static int umocktypes_are_equal_bool_ptr(bool** left, bool** right)
{
    int result;

    if (*left == *right)
    {
        result = 1;
    }
    else
    {
        if (*(*left) == *(*right))
        {
            result = 1;
        }
        else
        {
            result = 0;
        }
    }

    return result;
}


// bool FLOW_HANDLE functions

static int umocktypes_copy_FLOW_HANDLE(FLOW_HANDLE* destination, const FLOW_HANDLE* source)
{
    int result = 0;

    *(destination) = *(source);

    return result;
}

static void umocktypes_free_FLOW_HANDLE(FLOW_HANDLE* value)
{
    (void)value;
}

static char* umocktypes_stringify_FLOW_HANDLE(const FLOW_HANDLE* value)
{
    char temp_buffer[32];
    char* result;
    size_t length = sprintf(temp_buffer, "%p", (void*)*value);
    if (length < 0)
    {
        result = NULL;
    }
    else
    {
        result = (char*)malloc(length + 1);
        if (result != NULL)
        {
            (void)memcpy(result, temp_buffer, length + 1);
        }
    }
    return result;
}

static int umocktypes_are_equal_FLOW_HANDLE(FLOW_HANDLE* left, FLOW_HANDLE* right)
{
    int result;

    if (*left == *right)
    {
        result = 1;
    }
    else
    {
        result = 0;
    }

    return result;
}

// TRANSFER umock functions

static int umocktypes_copy_TRANSFER_HANDLE(TRANSFER_HANDLE* destination, const TRANSFER_HANDLE* source)
{
    int result = 0;

    *(destination) = *(source);

    return result;
}

static void umocktypes_free_TRANSFER_HANDLE(TRANSFER_HANDLE* value)
{
    (void)value;
}

static char* umocktypes_stringify_TRANSFER_HANDLE(const TRANSFER_HANDLE* value)
{
    char temp_buffer[32];
    char* result;
    size_t length = sprintf(temp_buffer, "%p", (void*)*value);
    if (length < 0)
    {
        result = NULL;
    }
    else
    {
        result = (char*)malloc(length + 1);
        if (result != NULL)
        {
            (void)memcpy(result, temp_buffer, length + 1);
        }
    }
    return result;
}

static int umocktypes_are_equal_TRANSFER_HANDLE(TRANSFER_HANDLE* left, TRANSFER_HANDLE* right)
{
    int result;

    if (*left == *right)
    {
        result = 1;
    }
    else
    {
        result = 0;
    }

    return result;
}

// delivery_tag mocks

static char* umock_stringify_delivery_tag(const delivery_tag* value)
{
    char* result = (char*)my_gballoc_malloc(1);
    (void)value;
    result[0] = '\0';
    return result;
}

static int umock_are_equal_delivery_tag(const delivery_tag* left, const delivery_tag* right)
{
    int result;

    if (left->length != right->length)
    {
        result = 0;
    }
    else
    {
        if (memcmp(left->bytes, right->bytes, left->length) == 0)
        {
            result = 1;
        }
        else
        {
            result = 0;
        }
    }

    return result;
}

static int umock_copy_delivery_tag(delivery_tag* destination, const delivery_tag* source)
{
    int result;

    destination->bytes = (const unsigned char*)my_gballoc_malloc(source->length);
    if (destination->bytes == NULL)
    {
        result = -1;
    }
    else
    {
        (void)memcpy((void*)destination->bytes, source->bytes, source->length);
        destination->length = source->length;
        result = 0;
    }

    return result;
}

static void umock_free_delivery_tag(delivery_tag* value)
{
    my_gballoc_free((void*)value->bytes);
    value->bytes = NULL;
    value->length = 0;
}


static TRANSFER_HANDLE test_on_transfer_received_transfer;
static uint32_t test_on_transfer_received_payload_size;
static unsigned char test_on_transfer_received_payload_bytes[2048];
static AMQP_VALUE test_on_transfer_received(void* context, TRANSFER_HANDLE transfer, uint32_t payload_size, const unsigned char* payload_bytes)
{
    (void)context;
    test_on_transfer_received_transfer = transfer;
    test_on_transfer_received_payload_size = payload_size;
    memcpy(test_on_transfer_received_payload_bytes, payload_bytes, payload_size);

    return (AMQP_VALUE)0x6000;
}

static LINK_STATE test_on_link_state_changed_new_link_state;
LINK_STATE test_on_link_state_changed_previous_link_state;
static void test_on_link_state_changed(void* context, LINK_STATE new_link_state, LINK_STATE previous_link_state)
{
    (void)context;
    test_on_link_state_changed_new_link_state = new_link_state;
    test_on_link_state_changed_previous_link_state = previous_link_state;
}

static void test_on_link_flow_on(void* context)
{
    (void)context;
}

static void on_delivery_settled(void* context, delivery_number delivery_no, LINK_DELIVERY_SETTLE_REASON reason, AMQP_VALUE delivery_state)
{
    (void)context;
    (void)delivery_no;
    (void)reason;
    (void)delivery_state;
}

static LINK_HANDLE create_link(role link_role)
{
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_clone(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_clone(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(tickcounter_create());
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(session_create_link_endpoint(TEST_SESSION_HANDLE, TEST_LINK_NAME_1));
    STRICT_EXPECTED_CALL(session_set_link_endpoint_callback(TEST_LINK_ENDPOINT, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    return link_create(TEST_SESSION_HANDLE, TEST_LINK_NAME_1, link_role, TEST_LINK_SOURCE, TEST_LINK_TARGET);
}

// If on_session_state_changed it does the full attach, including receiving the ATTACH response.
static int attach_link(LINK_HANDLE link, role link_role, ON_ENDPOINT_FRAME_RECEIVED* on_frame_received, ON_SESSION_STATE_CHANGED* on_session_state_changed)
{
    int result;

    umock_c_reset_all_calls();

    ATTACH_HANDLE attach = (ATTACH_HANDLE)0x4999;

    STRICT_EXPECTED_CALL(session_begin(TEST_SESSION_HANDLE));

    if (on_session_state_changed != NULL)
    {
        STRICT_EXPECTED_CALL(session_start_link_endpoint(TEST_LINK_ENDPOINT, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, link))
            .CaptureArgumentValue_frame_received_callback(on_frame_received)
            .CaptureArgumentValue_on_session_state_changed(on_session_state_changed);

        result = link_attach(link, test_on_transfer_received, test_on_link_state_changed, test_on_link_flow_on, NULL);
    }
    else
    {
        ON_SESSION_STATE_CHANGED local_on_session_state_changed = NULL;
        ON_ENDPOINT_FRAME_RECEIVED local_on_frame_received = NULL;
        AMQP_VALUE performative = (AMQP_VALUE)0x5000;
        AMQP_VALUE descriptor = (AMQP_VALUE)0x5001;
        uint32_t frame_payload_size = 30;
        const unsigned char payload_bytes[30] = { 0 };
        uint64_t max_message_size = 123456;

        STRICT_EXPECTED_CALL(session_start_link_endpoint(TEST_LINK_ENDPOINT, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, link))
            .CaptureArgumentValue_frame_received_callback(&local_on_frame_received)
            .CaptureArgumentValue_on_session_state_changed(&local_on_session_state_changed);

        result = link_attach(link, test_on_transfer_received, test_on_link_state_changed, test_on_link_flow_on, NULL);

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(attach_create(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .SetReturn(attach);
        STRICT_EXPECTED_CALL(attach_set_snd_settle_mode(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(attach_set_rcv_settle_mode(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(attach_set_role(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(attach_set_source(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(attach_set_target(IGNORED_PTR_ARG, IGNORED_PTR_ARG));

        if (link_role == role_sender)
        {
            STRICT_EXPECTED_CALL(attach_set_initial_delivery_count(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        }

        STRICT_EXPECTED_CALL(attach_set_max_message_size(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(session_send_attach(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(attach_destroy(IGNORED_PTR_ARG));

        local_on_session_state_changed(link, SESSION_STATE_MAPPED, SESSION_STATE_UNMAPPED);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        umock_c_reset_all_calls();
        // ATTACH response
        STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(performative))
            .SetReturn(descriptor);
        STRICT_EXPECTED_CALL(is_attach_type_by_descriptor(IGNORED_PTR_ARG))
            .SetReturn(true);
        STRICT_EXPECTED_CALL(amqpvalue_get_attach(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(attach_get_max_message_size(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
            .CopyOutArgumentBuffer(2, &max_message_size, sizeof(max_message_size));
        STRICT_EXPECTED_CALL(attach_destroy(IGNORED_PTR_ARG));

        local_on_frame_received(link, performative, frame_payload_size, payload_bytes);

        // FLOW received from remote endpoint.

        if (link_role == role_sender)
        {
            FLOW_HANDLE flow = (FLOW_HANDLE)0x5679;
            uint32_t link_credit = 10000;
            delivery_number delivery_count = 0;

            STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(performative))
                .SetReturn(descriptor);
            STRICT_EXPECTED_CALL(is_attach_type_by_descriptor(IGNORED_PTR_ARG))
                .SetReturn(false);
            STRICT_EXPECTED_CALL(is_flow_type_by_descriptor(IGNORED_PTR_ARG))
                .SetReturn(true);
            STRICT_EXPECTED_CALL(amqpvalue_get_flow(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
                .CopyOutArgumentBuffer(2, &flow, sizeof(flow));
            STRICT_EXPECTED_CALL(flow_get_link_credit(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
                .CopyOutArgumentBuffer(2, &link_credit, sizeof(link_credit))
                .SetReturn(0);
            STRICT_EXPECTED_CALL(flow_get_delivery_count(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
                .CopyOutArgumentBuffer(2, &delivery_count, sizeof(delivery_count))
                .SetReturn(0);
            STRICT_EXPECTED_CALL(flow_destroy(IGNORED_PTR_ARG));

            local_on_frame_received(link, performative, frame_payload_size, payload_bytes);
        }

        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        if (on_frame_received != NULL)
        {
            *on_frame_received = local_on_frame_received;
        }
    }

    return result;
}

BEGIN_TEST_SUITE(link_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;

    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    result = umocktypes_bool_register_types();
    ASSERT_ARE_EQUAL(int, 0, result, "Failed registering bool types");

    result = umocktypes_stdint_register_types();
    ASSERT_ARE_EQUAL(int, 0, result, "Failed registering stdint types");

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_calloc, my_gballoc_calloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_realloc, my_gballoc_realloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
    REGISTER_UMOCK_ALIAS_TYPE(transfer_number, uint32_t);
    REGISTER_UMOCK_ALIAS_TYPE(role, bool);
    REGISTER_UMOCK_ALIAS_TYPE(delivery_number, uint32_t);
    REGISTER_UMOCK_ALIAS_TYPE(sequence_no, uint32_t);
    REGISTER_UMOCK_ALIAS_TYPE(handle, uint32_t);
    REGISTER_UMOCK_ALIAS_TYPE(sender_settle_mode, uint8_t);
    REGISTER_UMOCK_ALIAS_TYPE(receiver_settle_mode, uint8_t);
    REGISTER_UMOCK_ALIAS_TYPE(DISPOSITION_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(AMQP_VALUE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(TICK_COUNTER_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(SINGLYLINKEDLIST_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(LIST_ITEM_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(SESSION_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ATTACH_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(LINK_ENDPOINT_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_LINK_ENDPOINT_DESTROYED_CALLBACK, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_ENDPOINT_FRAME_RECEIVED, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_TRANSFER_RECEIVED, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_LINK_STATE_CHANGED, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_LINK_FLOW_ON, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_SESSION_STATE_CHANGED, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_SESSION_FLOW_ON, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ASYNC_OPERATION_CANCEL_HANDLER_FUNC, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ASYNC_OPERATION_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(message_format, uint32_t);
    REGISTER_UMOCK_ALIAS_TYPE(ON_SEND_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(SESSION_SEND_TRANSFER_RESULT, int);

    REGISTER_GLOBAL_MOCK_RETURNS(tickcounter_create, TEST_TICK_COUNTER_HANDLE, NULL);
    REGISTER_GLOBAL_MOCK_RETURNS(singlylinkedlist_create, TEST_SINGLYLINKEDLIST_HANDLE, NULL);
    REGISTER_GLOBAL_MOCK_RETURNS(session_create_link_endpoint, TEST_LINK_ENDPOINT, NULL);
    REGISTER_GLOBAL_MOCK_RETURNS(session_start_link_endpoint, 0, 1);

    REGISTER_GLOBAL_MOCK_FAIL_RETURN(async_operation_create, NULL);
    REGISTER_GLOBAL_MOCK_RETURNS(transfer_create, TEST_TRANSFER_HANDLE, NULL);
    REGISTER_GLOBAL_MOCK_RETURNS(transfer_set_delivery_tag, 0, 1);
    REGISTER_GLOBAL_MOCK_RETURNS(transfer_set_message_format, 0, 1);
    REGISTER_GLOBAL_MOCK_RETURNS(transfer_set_settled, 0, 1);
    REGISTER_GLOBAL_MOCK_RETURNS(amqpvalue_create_transfer, TEST_AMQP_VALUE, NULL);
    REGISTER_GLOBAL_MOCK_RETURNS(tickcounter_get_current_ms, 0, 1);
    REGISTER_GLOBAL_MOCK_RETURNS(singlylinkedlist_add, TEST_LIST_ITEM_HANDLE, NULL);
    REGISTER_GLOBAL_MOCK_RETURNS(singlylinkedlist_remove, 0, 1);
    REGISTER_GLOBAL_MOCK_RETURNS(session_send_transfer, 0, 1);

    REGISTER_TYPE(FLOW_HANDLE, FLOW_HANDLE);
    REGISTER_TYPE(TRANSFER_HANDLE, TRANSFER_HANDLE);
    REGISTER_TYPE(bool*, bool_ptr);
    REGISTER_TYPE(uint32_t, uint32_t);

    REGISTER_UMOCK_VALUE_TYPE(delivery_tag);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();

    TEST_MUTEX_DESTROY(g_testByTest);
}

TEST_FUNCTION_INITIALIZE(test_init)
{
    if (TEST_MUTEX_ACQUIRE(g_testByTest))
    {
        ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
    }

    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(test_cleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

TEST_FUNCTION(link_create_succeeds)
{
    // arrange
    AMQP_VALUE link_source = TEST_LINK_SOURCE;
    AMQP_VALUE link_target = TEST_LINK_TARGET;

    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_clone(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_clone(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(tickcounter_create());
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(session_create_link_endpoint(TEST_SESSION_HANDLE, TEST_LINK_NAME_1));
    STRICT_EXPECTED_CALL(session_set_link_endpoint_callback(TEST_LINK_ENDPOINT, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    LINK_HANDLE link = link_create(TEST_SESSION_HANDLE, TEST_LINK_NAME_1, role_receiver, link_source, link_target);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(link);

    // cleanup
    link_destroy(link);
}

TEST_FUNCTION(link_attach_succeeds)
{
    // arrange
    LINK_HANDLE link = create_link(role_receiver);
    ON_ENDPOINT_FRAME_RECEIVED on_frame_received = NULL;

    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(session_begin(TEST_SESSION_HANDLE));
    STRICT_EXPECTED_CALL(session_start_link_endpoint(TEST_LINK_ENDPOINT, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, link))
        .CaptureArgumentValue_frame_received_callback(&on_frame_received);

    // act
    int result = link_attach(link, test_on_transfer_received, test_on_link_state_changed, test_on_link_flow_on, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_IS_NOT_NULL(on_frame_received);

    // cleanup
    link_destroy(link);
}

TEST_FUNCTION(link_receiver_frame_received_succeeds)
{
    // arrange
    LINK_HANDLE link = create_link(role_receiver);
    ON_ENDPOINT_FRAME_RECEIVED on_frame_received = NULL;
    ON_SESSION_STATE_CHANGED on_session_state_changed = NULL;
    int attach_result = attach_link(link, role_receiver, &on_frame_received, &on_session_state_changed);
    ASSERT_ARE_EQUAL(int, 0, attach_result);

    AMQP_VALUE performative = (AMQP_VALUE)0x5000;
    AMQP_VALUE descriptor = (AMQP_VALUE)0x5001;
    FLOW_HANDLE flow = (FLOW_HANDLE)0x5002;
    uint32_t frame_payload_size = 30;
    const unsigned char payload_bytes[30] = { 0 };

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(performative))
        .SetReturn(descriptor);
    STRICT_EXPECTED_CALL(is_attach_type_by_descriptor(IGNORED_PTR_ARG))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_flow_type_by_descriptor(IGNORED_PTR_ARG))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(amqpvalue_get_flow(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &flow, sizeof(flow));
    STRICT_EXPECTED_CALL(flow_destroy(IGNORED_PTR_ARG));

    // act
    on_frame_received(link, performative, frame_payload_size, payload_bytes);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    link_destroy(link);
}

TEST_FUNCTION(link_sender_frame_received_succeeds)
{
    // arrange
    LINK_HANDLE link = create_link(role_sender);
    ON_ENDPOINT_FRAME_RECEIVED on_frame_received = NULL;
    ON_SESSION_STATE_CHANGED on_session_state_changed = NULL;
    int attach_result = attach_link(link, role_sender, &on_frame_received, &on_session_state_changed);
    ASSERT_ARE_EQUAL(int, 0, attach_result);

    AMQP_VALUE performative = (AMQP_VALUE)0x5000;
    AMQP_VALUE descriptor = (AMQP_VALUE)0x5001;
    FLOW_HANDLE flow = (FLOW_HANDLE)0x5002;
    uint32_t frame_payload_size = 30;
    const unsigned char payload_bytes[30] = { 0 };
    uint32_t link_credit_value = 700;
    uint32_t delivery_count_value = 300;

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(performative))
        .SetReturn(descriptor);
    STRICT_EXPECTED_CALL(is_attach_type_by_descriptor(IGNORED_PTR_ARG))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_flow_type_by_descriptor(IGNORED_PTR_ARG))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(amqpvalue_get_flow(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &flow, sizeof(flow));
    STRICT_EXPECTED_CALL(flow_get_link_credit(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &link_credit_value, sizeof(link_credit_value));
    STRICT_EXPECTED_CALL(flow_get_delivery_count(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &delivery_count_value, sizeof(delivery_count_value));
    STRICT_EXPECTED_CALL(flow_destroy(IGNORED_PTR_ARG));

    // act
    on_frame_received(link, performative, frame_payload_size, payload_bytes);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    link_destroy(link);
}

TEST_FUNCTION(link_receiver_frame_received_get_flow_fails_no_double_free_fails)
{
    // arrange
    LINK_HANDLE link = create_link(role_receiver);
    ON_ENDPOINT_FRAME_RECEIVED on_frame_received = NULL;
    ON_SESSION_STATE_CHANGED on_session_state_changed = NULL;
    int attach_result = attach_link(link, role_receiver, &on_frame_received, &on_session_state_changed);
    ASSERT_ARE_EQUAL(int, 0, attach_result);

    AMQP_VALUE performative = (AMQP_VALUE)0x5000;
    AMQP_VALUE descriptor = (AMQP_VALUE)0x5001;
    FLOW_HANDLE flow = NULL;
    uint32_t frame_payload_size = 30;
    const unsigned char payload_bytes[30] = { 0 };

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(performative))
        .SetReturn(descriptor);
    STRICT_EXPECTED_CALL(is_attach_type_by_descriptor(IGNORED_PTR_ARG))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_flow_type_by_descriptor(IGNORED_PTR_ARG))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(amqpvalue_get_flow(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &flow, sizeof(flow))
        .SetReturn(1);

    // act
    on_frame_received(link, performative, frame_payload_size, payload_bytes);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    link_destroy(link);
}

TEST_FUNCTION(link_receiver_default_max_credit_succeeds)
{
    // arrange
    const int DEFAULT_LINK_CREDIT = 10000;

    LINK_HANDLE link = create_link(role_receiver);

    ON_ENDPOINT_FRAME_RECEIVED on_frame_received = NULL;
    ON_SESSION_STATE_CHANGED on_session_state_changed = NULL;
    int attach_result = attach_link(link, role_receiver, &on_frame_received, &on_session_state_changed);
    ASSERT_ARE_EQUAL(int, 0, attach_result);

    ATTACH_HANDLE attach = (ATTACH_HANDLE)0x4999;
    AMQP_VALUE performative = (AMQP_VALUE)0x5000;
    AMQP_VALUE descriptor = (AMQP_VALUE)0x5001;
    FLOW_HANDLE flow = (FLOW_HANDLE)0x5002;
    TRANSFER_HANDLE transfer = (TRANSFER_HANDLE)0x5003;
    uint32_t frame_payload_size = 30;
    const unsigned char payload_bytes[30] = { 0 };
    bool more = false;
    DISPOSITION_HANDLE disposition = (DISPOSITION_HANDLE)0x5005;

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(attach_create(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .SetReturn(attach);
    STRICT_EXPECTED_CALL(attach_set_snd_settle_mode(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(attach_set_rcv_settle_mode(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(attach_set_role(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(attach_set_source(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(attach_set_target(IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(attach_set_max_message_size(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(session_send_attach(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(attach_destroy(IGNORED_PTR_ARG));

    on_session_state_changed(link, SESSION_STATE_MAPPED, SESSION_STATE_UNMAPPED);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(performative))
        .SetReturn(descriptor);
    STRICT_EXPECTED_CALL(is_attach_type_by_descriptor(IGNORED_PTR_ARG))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_flow_type_by_descriptor(IGNORED_PTR_ARG))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(amqpvalue_get_flow(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &flow, sizeof(flow));
    STRICT_EXPECTED_CALL(flow_destroy(IGNORED_PTR_ARG));
    on_frame_received(link, performative, frame_payload_size, payload_bytes);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    umock_c_reset_all_calls();
    // First transfer results in FLOW to set link credit.
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(performative))
        .SetReturn(descriptor);
    STRICT_EXPECTED_CALL(is_attach_type_by_descriptor(IGNORED_PTR_ARG))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_flow_type_by_descriptor(IGNORED_PTR_ARG))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_transfer_type_by_descriptor(IGNORED_PTR_ARG))
        .SetReturn(true);
    STRICT_EXPECTED_CALL(amqpvalue_get_transfer(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &transfer, sizeof(transfer));
    // send_flow
    STRICT_EXPECTED_CALL(flow_create(IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .SetReturn(flow);
    STRICT_EXPECTED_CALL(flow_set_link_credit(IGNORED_PTR_ARG, DEFAULT_LINK_CREDIT));
    STRICT_EXPECTED_CALL(flow_set_handle(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(flow_set_delivery_count(IGNORED_PTR_ARG, 0));
    STRICT_EXPECTED_CALL(session_send_flow(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(flow_destroy(IGNORED_PTR_ARG));
    // continue processing TRANSFER
    STRICT_EXPECTED_CALL(transfer_get_more(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &more, sizeof(bool));
    STRICT_EXPECTED_CALL(transfer_get_delivery_id(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(disposition_create(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .SetReturn(disposition);
    STRICT_EXPECTED_CALL(disposition_set_last(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(disposition_set_settled(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(disposition_set_state(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(session_send_disposition(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(disposition_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(transfer_destroy(IGNORED_PTR_ARG));

    // act
    on_frame_received(link, performative, frame_payload_size, payload_bytes);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    link_destroy(link);
}

TEST_FUNCTION(link_receiver_link_credit_replenish_succeeds)
{
    // arrange
    LINK_HANDLE link = create_link(role_receiver);
    ASSERT_ARE_EQUAL(int, 0, link_set_max_link_credit(link, 5));

    ON_ENDPOINT_FRAME_RECEIVED on_frame_received = NULL;
    ON_SESSION_STATE_CHANGED on_session_state_changed = NULL;
    int attach_result = attach_link(link, role_receiver, &on_frame_received, &on_session_state_changed);
    ASSERT_ARE_EQUAL(int, 0, attach_result);

    ATTACH_HANDLE attach = (ATTACH_HANDLE)0x4999;
    AMQP_VALUE performative = (AMQP_VALUE)0x5000;
    AMQP_VALUE descriptor = (AMQP_VALUE)0x5001;
    FLOW_HANDLE flow = (FLOW_HANDLE)0x5002;
    TRANSFER_HANDLE transfer = (TRANSFER_HANDLE)0x5003;
    uint32_t frame_payload_size = 30;
    const unsigned char payload_bytes[30] = { 0 };
    bool more = false;
    DISPOSITION_HANDLE disposition = (DISPOSITION_HANDLE)0x5005;

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(attach_create(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .SetReturn(attach);
    STRICT_EXPECTED_CALL(attach_set_snd_settle_mode(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(attach_set_rcv_settle_mode(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(attach_set_role(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(attach_set_source(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(attach_set_target(IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(attach_set_max_message_size(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(session_send_attach(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(attach_destroy(IGNORED_PTR_ARG));

    on_session_state_changed(link, SESSION_STATE_MAPPED, SESSION_STATE_UNMAPPED);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(performative))
        .SetReturn(descriptor);
    STRICT_EXPECTED_CALL(is_attach_type_by_descriptor(IGNORED_PTR_ARG))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_flow_type_by_descriptor(IGNORED_PTR_ARG))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(amqpvalue_get_flow(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &flow, sizeof(flow));
    STRICT_EXPECTED_CALL(flow_destroy(IGNORED_PTR_ARG));
    on_frame_received(link, performative, frame_payload_size, payload_bytes);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    umock_c_reset_all_calls();
    // First transfer results in FLOW to set link credit.
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(performative))
        .SetReturn(descriptor);
    STRICT_EXPECTED_CALL(is_attach_type_by_descriptor(IGNORED_PTR_ARG))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_flow_type_by_descriptor(IGNORED_PTR_ARG))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_transfer_type_by_descriptor(IGNORED_PTR_ARG))
        .SetReturn(true);
    STRICT_EXPECTED_CALL(amqpvalue_get_transfer(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &transfer, sizeof(transfer));
    // send_flow
    STRICT_EXPECTED_CALL(flow_create(IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .SetReturn(flow);
    STRICT_EXPECTED_CALL(flow_set_link_credit(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(flow_set_handle(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(flow_set_delivery_count(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(session_send_flow(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(flow_destroy(IGNORED_PTR_ARG));
    // continue processing TRANSFER
    STRICT_EXPECTED_CALL(transfer_get_more(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &more, sizeof(bool));
    STRICT_EXPECTED_CALL(transfer_get_delivery_id(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(disposition_create(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .SetReturn(disposition);
    STRICT_EXPECTED_CALL(disposition_set_last(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(disposition_set_settled(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(disposition_set_state(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(session_send_disposition(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(disposition_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(transfer_destroy(IGNORED_PTR_ARG));
    on_frame_received(link, performative, frame_payload_size, payload_bytes);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    umock_c_reset_all_calls();

    // act
    // First 3 transfers result in no FLOW.
    for (int i = 0; i < 3; i++)
    {
        STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(performative))
            .SetReturn(descriptor);
        STRICT_EXPECTED_CALL(is_attach_type_by_descriptor(IGNORED_PTR_ARG))
            .SetReturn(false);
        STRICT_EXPECTED_CALL(is_flow_type_by_descriptor(IGNORED_PTR_ARG))
            .SetReturn(false);
        STRICT_EXPECTED_CALL(is_transfer_type_by_descriptor(IGNORED_PTR_ARG))
            .SetReturn(true);
        STRICT_EXPECTED_CALL(amqpvalue_get_transfer(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
            .CopyOutArgumentBuffer(2, &transfer, sizeof(transfer));
        STRICT_EXPECTED_CALL(transfer_get_more(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
            .CopyOutArgumentBuffer(2, &more, sizeof(bool));
        STRICT_EXPECTED_CALL(transfer_get_delivery_id(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(disposition_create(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .SetReturn(disposition);
        STRICT_EXPECTED_CALL(disposition_set_last(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(disposition_set_settled(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(disposition_set_state(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(session_send_disposition(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(disposition_destroy(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(amqpvalue_destroy(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(transfer_destroy(IGNORED_PTR_ARG));
        on_frame_received(link, performative, frame_payload_size, payload_bytes);
    }

    // And 4th does result in FLOW.
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(performative))
        .SetReturn(descriptor);
    STRICT_EXPECTED_CALL(is_attach_type_by_descriptor(IGNORED_PTR_ARG))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_flow_type_by_descriptor(IGNORED_PTR_ARG))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_transfer_type_by_descriptor(IGNORED_PTR_ARG))
        .SetReturn(true);
    STRICT_EXPECTED_CALL(amqpvalue_get_transfer(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &transfer, sizeof(transfer));
    // send_flow
    STRICT_EXPECTED_CALL(flow_create(IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .SetReturn(flow);
    STRICT_EXPECTED_CALL(flow_set_link_credit(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(flow_set_handle(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(flow_set_delivery_count(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(session_send_flow(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(flow_destroy(IGNORED_PTR_ARG));
    // continue processing TRANSFER
    STRICT_EXPECTED_CALL(transfer_get_more(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &more, sizeof(bool));
    STRICT_EXPECTED_CALL(transfer_get_delivery_id(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(disposition_create(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .SetReturn(disposition);
    STRICT_EXPECTED_CALL(disposition_set_last(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(disposition_set_settled(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(disposition_set_state(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(session_send_disposition(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(disposition_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(transfer_destroy(IGNORED_PTR_ARG));
    on_frame_received(link, performative, frame_payload_size, payload_bytes);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    link_destroy(link);
}

TEST_FUNCTION(link_transfer_async_success)
{
    // arrange
    LINK_HANDLE link = create_link(role_sender);

    ON_ENDPOINT_FRAME_RECEIVED on_frame_received = NULL;
    LINK_TRANSFER_RESULT link_transfer_error;
    tickcounter_ms_t send_timeout = 1000;
    uint8_t data_bytes[16];
    PAYLOAD payload;
    payload.bytes = (const unsigned char*)data_bytes;
    payload.length = 0;
    const size_t message_count = 1;
    uint8_t async_op_result[128];
    unsigned char delivery_tag_bytes[10];
    delivery_tag moot_delivery_tag;
    moot_delivery_tag.bytes = delivery_tag_bytes;
    moot_delivery_tag.length = sizeof(delivery_tag_bytes);

    int attach_result = attach_link(link, role_sender, &on_frame_received, NULL);
    ASSERT_ARE_EQUAL(int, 0, attach_result);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(async_operation_create(IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn((ASYNC_OPERATION_HANDLE)async_op_result);
    STRICT_EXPECTED_CALL(transfer_create(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(transfer_set_delivery_tag(IGNORED_PTR_ARG, moot_delivery_tag))
        .IgnoreArgument_delivery_tag_value(); // Important for making umock-c work with custom value arguments.
    STRICT_EXPECTED_CALL(transfer_set_message_format(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(transfer_set_settled(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_create_transfer(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(tickcounter_get_current_ms(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(session_send_transfer(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(SESSION_SEND_TRANSFER_OK);
    STRICT_EXPECTED_CALL(amqpvalue_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(transfer_destroy(IGNORED_PTR_ARG));

    // act
    ASYNC_OPERATION_HANDLE async_result = link_transfer_async(link, AMQP_BATCHING_FORMAT_CODE, &payload, message_count, on_delivery_settled, NULL, &link_transfer_error, send_timeout);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(async_result);

    // cleanup
    link_destroy(link);
}

TEST_FUNCTION(link_transfer_async_SESSION_SEND_TRANSFER_ERROR_fails)
{
    // arrange
    LINK_HANDLE link = create_link(role_sender);

    ON_ENDPOINT_FRAME_RECEIVED on_frame_received = NULL;
    LINK_TRANSFER_RESULT link_transfer_error;
    tickcounter_ms_t send_timeout = 1000;
    uint8_t data_bytes[16];
    PAYLOAD payload;
    payload.bytes = (const unsigned char*)data_bytes;
    payload.length = 0;
    const size_t message_count = 1;
    uint8_t async_op_result[128];
    unsigned char delivery_tag_bytes[10];
    delivery_tag moot_delivery_tag;
    moot_delivery_tag.bytes = delivery_tag_bytes;
    moot_delivery_tag.length = sizeof(delivery_tag_bytes);

    int attach_result = attach_link(link, role_sender, &on_frame_received, NULL);
    ASSERT_ARE_EQUAL(int, 0, attach_result);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(async_operation_create(IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn((ASYNC_OPERATION_HANDLE)async_op_result);
    STRICT_EXPECTED_CALL(transfer_create(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(transfer_set_delivery_tag(IGNORED_PTR_ARG, moot_delivery_tag))
        .IgnoreArgument_delivery_tag_value(); // Important for making umock-c work with custom value arguments.
    STRICT_EXPECTED_CALL(transfer_set_message_format(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(transfer_set_settled(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_create_transfer(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(tickcounter_get_current_ms(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(session_send_transfer(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(SESSION_SEND_TRANSFER_ERROR);
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(async_operation_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(transfer_destroy(IGNORED_PTR_ARG));

    // act
    ASYNC_OPERATION_HANDLE async_result = link_transfer_async(link, AMQP_BATCHING_FORMAT_CODE, &payload, message_count, on_delivery_settled, NULL, &link_transfer_error, send_timeout);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(async_result);

    // cleanup
    link_destroy(link);
}

TEST_FUNCTION(link_transfer_async_SESSION_SEND_TRANSFER_BUSY_fails)
{
    // arrange
    LINK_HANDLE link = create_link(role_sender);

    ON_ENDPOINT_FRAME_RECEIVED on_frame_received = NULL;
    LINK_TRANSFER_RESULT link_transfer_error;
    tickcounter_ms_t send_timeout = 1000;
    uint8_t data_bytes[16];
    PAYLOAD payload;
    payload.bytes = (const unsigned char*)data_bytes;
    payload.length = 0;
    const size_t message_count = 1;
    uint8_t async_op_result[128];
    unsigned char delivery_tag_bytes[10];
    delivery_tag moot_delivery_tag;
    moot_delivery_tag.bytes = delivery_tag_bytes;
    moot_delivery_tag.length = sizeof(delivery_tag_bytes);

    int attach_result = attach_link(link, role_sender, &on_frame_received, NULL);
    ASSERT_ARE_EQUAL(int, 0, attach_result);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(async_operation_create(IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn((ASYNC_OPERATION_HANDLE)async_op_result);
    STRICT_EXPECTED_CALL(transfer_create(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(transfer_set_delivery_tag(IGNORED_PTR_ARG, moot_delivery_tag))
        .IgnoreArgument_delivery_tag_value(); // Important for making umock-c work with custom value arguments.
    STRICT_EXPECTED_CALL(transfer_set_message_format(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(transfer_set_settled(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_create_transfer(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(tickcounter_get_current_ms(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(session_send_transfer(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(SESSION_SEND_TRANSFER_BUSY);
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(async_operation_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(transfer_destroy(IGNORED_PTR_ARG));

    // act
    ASYNC_OPERATION_HANDLE async_result = link_transfer_async(link, AMQP_BATCHING_FORMAT_CODE, &payload, message_count, on_delivery_settled, NULL, &link_transfer_error, send_timeout);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(async_result);

    // cleanup
    link_destroy(link);
}

// If `session_send_transfer` fails internally, depending on the point where it fails `remove_all_pending_deliveries` might be called,
// causing the ASYNC_OPERATION_HANDLE/DELIVERY_INSTANCE variable `result` to be removed from `link->pending_deliveries` and destroyed.
// Then when `session_send_transfer` returns, in such cases `singlylinkedlist_remove` will fail and `async_operation_destroy` should not be called (to avoid a double-free).
TEST_FUNCTION(link_transfer_async_SESSION_SEND_TRANSFER_ERROR_result_already_destroyed_fails)
{
    // arrange
    LINK_HANDLE link = create_link(role_sender);

    ON_ENDPOINT_FRAME_RECEIVED on_frame_received = NULL;
    LINK_TRANSFER_RESULT link_transfer_error;
    tickcounter_ms_t send_timeout = 1000;
    uint8_t data_bytes[16];
    PAYLOAD payload;
    payload.bytes = (const unsigned char*)data_bytes;
    payload.length = 0;
    const size_t message_count = 1;
    uint8_t async_op_result[128];
    unsigned char delivery_tag_bytes[10];
    delivery_tag moot_delivery_tag;
    moot_delivery_tag.bytes = delivery_tag_bytes;
    moot_delivery_tag.length = sizeof(delivery_tag_bytes);

    int attach_result = attach_link(link, role_sender, &on_frame_received, NULL);
    ASSERT_ARE_EQUAL(int, 0, attach_result);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(async_operation_create(IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn((ASYNC_OPERATION_HANDLE)async_op_result);
    STRICT_EXPECTED_CALL(transfer_create(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(transfer_set_delivery_tag(IGNORED_PTR_ARG, moot_delivery_tag))
        .IgnoreArgument_delivery_tag_value(); // Important for making umock-c work with custom value arguments.
    STRICT_EXPECTED_CALL(transfer_set_message_format(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(transfer_set_settled(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_create_transfer(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(tickcounter_get_current_ms(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(session_send_transfer(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(SESSION_SEND_TRANSFER_ERROR);
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(1); // result has already been removed from link->pending_deliveries.
    // Do not expect async_operation_destroy!
    STRICT_EXPECTED_CALL(amqpvalue_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(transfer_destroy(IGNORED_PTR_ARG));

    // act
    ASYNC_OPERATION_HANDLE async_result = link_transfer_async(link, AMQP_BATCHING_FORMAT_CODE, &payload, message_count, on_delivery_settled, NULL, &link_transfer_error, send_timeout);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(async_result);

    // cleanup
    link_destroy(link);
}

// If `session_send_transfer` fails internally, depending on the point where it fails `remove_all_pending_deliveries` might be called,
// causing the ASYNC_OPERATION_HANDLE/DELIVERY_INSTANCE variable `result` to be removed from `link->pending_deliveries` and destroyed.
// Then when `session_send_transfer` returns, in such cases `singlylinkedlist_remove` will fail and `async_operation_destroy` should not be called (to avoid a double-free).
TEST_FUNCTION(link_transfer_async_SESSION_SEND_TRANSFER_BUSY_result_already_destroyed_fails)
{
    // arrange
    LINK_HANDLE link = create_link(role_sender);

    ON_ENDPOINT_FRAME_RECEIVED on_frame_received = NULL;
    LINK_TRANSFER_RESULT link_transfer_error;
    tickcounter_ms_t send_timeout = 1000;
    uint8_t data_bytes[16];
    PAYLOAD payload;
    payload.bytes = (const unsigned char*)data_bytes;
    payload.length = 0;
    const size_t message_count = 1;
    uint8_t async_op_result[128];
    unsigned char delivery_tag_bytes[10];
    delivery_tag moot_delivery_tag;
    moot_delivery_tag.bytes = delivery_tag_bytes;
    moot_delivery_tag.length = sizeof(delivery_tag_bytes);

    int attach_result = attach_link(link, role_sender, &on_frame_received, NULL);
    ASSERT_ARE_EQUAL(int, 0, attach_result);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(async_operation_create(IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn((ASYNC_OPERATION_HANDLE)async_op_result);
    STRICT_EXPECTED_CALL(transfer_create(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(transfer_set_delivery_tag(IGNORED_PTR_ARG, moot_delivery_tag))
        .IgnoreArgument_delivery_tag_value(); // Important for making umock-c work with custom value arguments.
    STRICT_EXPECTED_CALL(transfer_set_message_format(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(transfer_set_settled(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_create_transfer(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(tickcounter_get_current_ms(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(session_send_transfer(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(SESSION_SEND_TRANSFER_BUSY);
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(1); // result has already been removed from link->pending_deliveries.
    // Do not expect async_operation_destroy!
    STRICT_EXPECTED_CALL(amqpvalue_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(transfer_destroy(IGNORED_PTR_ARG));

    // act
    ASYNC_OPERATION_HANDLE async_result = link_transfer_async(link, AMQP_BATCHING_FORMAT_CODE, &payload, message_count, on_delivery_settled, NULL, &link_transfer_error, send_timeout);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(async_result);

    // cleanup
    link_destroy(link);
}

END_TEST_SUITE(link_ut)
