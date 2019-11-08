/*******************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only
* intended for use with Renesas products. No other uses are authorized. This
* software is owned by Renesas Electronics Corporation and is protected under
* all applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT
* LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
* AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.
* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS
* ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR
* ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE
* BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software
* and to discontinue the availability of this software. By using this software,
* you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2019 Renesas Electronics Corporation. All rights reserved.
*******************************************************************************/
#include "sample_select.h"

#if (SAMPLE_PROGRAM_NO == 13)
// SAMPLE_PROGRAM_NO 13 : Ether HTTP sample

#if defined(TARGET_SEMB1402)
#error "Ether is not supported."
#endif

#include "mbed.h"
#include "http_request.h"
#include "EthernetInterface.h"

EthernetInterface eth;

void dump_response(HttpResponse* res) {
    printf("Status: %d - %s\n", res->get_status_code(), res->get_status_message().c_str());

    printf("Headers:\n");
    for (size_t ix = 0; ix < res->get_headers_length(); ix++) {
        printf("\t%s: %s\n", res->get_headers_fields()[ix]->c_str(), res->get_headers_values()[ix]->c_str());
    }
    printf("\nBody (%d bytes):\n\n%s\n", res->get_body_length(), res->get_body_as_string().c_str());
}

int main() {
    NetworkInterface* network = &eth;

    printf("\nConnecting...\n");
    int ret = network->connect();
    if (ret != 0) {
        printf("\nConnection error\n");
        return -1;
    }
    printf("Success\n\n");
    printf("MAC: %s\n", network->get_mac_address());
    printf("IP: %s\n", network->get_ip_address());
    printf("Netmask: %s\n", network->get_netmask());
    printf("Gateway: %s\n", network->get_gateway());

    // Do a GET request to httpbin.org
    {
        // By default the body is automatically parsed and stored in a buffer, this is memory heavy.
        // To receive chunked response, pass in a callback as last parameter to the constructor.
        HttpRequest* get_req = new HttpRequest(network, HTTP_GET, "http://httpbin.org/status/418");

        HttpResponse* get_res = get_req->send();
        if (!get_res) {
            printf("HttpRequest failed (error code %d)\n", get_req->get_error());
            return 1;
        }

        printf("\n----- HTTP GET response -----\n");
        dump_response(get_res);

        delete get_req;
    }

    // POST request to httpbin.org
    {
        HttpRequest* post_req = new HttpRequest(network, HTTP_POST, "http://httpbin.org/post");
        post_req->set_header("Content-Type", "application/json");

        const char body[] = "{\"hello\":\"world\"}";

        HttpResponse* post_res = post_req->send(body, strlen(body));
        if (!post_res) {
            printf("HttpRequest failed (error code %d)\n", post_req->get_error());
            return 1;
        }

        printf("\n----- HTTP POST response -----\n");
        dump_response(post_res);

        delete post_req;
    }

    wait(osWaitForever);
}

#endif
