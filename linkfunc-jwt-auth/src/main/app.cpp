
#include <json/json.h>

#include <thread>
#include <fstream>
#include <sstream>
#include <iomanip>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <l8w8jwt/claim.h>
#include <l8w8jwt/encode.h>
#include <l8w8jwt/decode.h>
#include <l8w8jwt/retcodes.h>
#include <chillbuff.h>
#include <ngx_link_func_module.h>

#include <sys/stat.h>
#include <fcntl.h>

static Json::Value appConfiguration;

static int decode(char* KEY, char* JWT, ngx_link_func_ctx_t* ctx, std::string *jsonResult) {
    struct l8w8jwt_decoding_params params;
    l8w8jwt_decoding_params_init(&params);

    params.alg = L8W8JWT_ALG_HS256;

    params.jwt = (char*)JWT;
    params.jwt_length = strlen(JWT);

    params.verification_key = (unsigned char*)KEY;
    params.verification_key_length = strlen(KEY);

    params.validate_iss = (char*)appConfiguration["iss"].asCString();
    params.validate_iss_length = strlen(params.validate_iss);

    params.validate_sub = (char*)appConfiguration["sub"].asCString();
    params.validate_sub_length = strlen(params.validate_sub);

    params.validate_exp = true;
    params.exp_tolerance_seconds = 60;

    params.validate_iat = true;
    params.iat_tolerance_seconds = 60;

    enum l8w8jwt_validation_result validation_result;
    struct l8w8jwt_claim *out_claims;
    size_t out_claims_length;
    int r = l8w8jwt_decode(&params, &validation_result, &out_claims, &out_claims_length);

    // printf("\nl8w8jwt_decode_hs256 function returned %s (code %d).\n\nValidation result: \n%d\n", r == L8W8JWT_SUCCESS ? "successfully" : "", r, validation_result);
    // int i;
    // for (i = 0; i < out_claims_length; i++) {
    //     printf("key %s\n", out_claims[i].key);
    // }

    // chillbuff_init(jsonResult, 1024, sizeof(char), CHILLBUFF_GROW_DUPLICATIVE);

    struct l8w8jwt_claim *found_claim;

    // if ( L8W8JWT_SUCCESS == l8w8jwt_write_claims(jsonResult, out_claims, out_claims_length)) {
    // ngx_link_func_log(info, ctx, "Finding Claim");

    if ( r == L8W8JWT_SUCCESS && (found_claim = l8w8jwt_get_claim(out_claims, out_claims_length, "data", sizeof("data") - 1 ) ) )  {
        // printf("%s \n", jsonResult.array);
        // printf("String length: %d \n", (int)jsonResult.length);
        // ngx_link_func_log(info, ctx, "Claim Found");

        jsonResult->append((char*)found_claim->value, found_claim->value_length);

        // ngx_link_func_log(info, ctx, "Json Result: %.*s \n", (int)found_claim->value_length, (char*)found_claim->value);
        l8w8jwt_free_claims(out_claims, out_claims_length);
        // chillbuff_free(&jsonResult);
        return 0;
    } else {
        l8w8jwt_free_claims(out_claims, out_claims_length);
        return -1;
    }
}

void verifyJwt(ngx_link_func_ctx_t* ctx) {
    char* jwt = (char*) ngx_link_func_get_header(ctx, "Authorization", sizeof("Authorization") - 1);

    if (jwt) {
        ngx_link_func_log(info, ctx, "proceed with token:\n%s", jwt);
    }

    if (jwt && strncmp("Bearer ", jwt, sizeof("Bearer ") - 1) == 0) {
        jwt = jwt + (sizeof("Bearer ") - 1);
        ngx_link_func_log(info, ctx, "json web token:\n%s", jwt);

        std::string result;
        /** Add output header for downstream response **/
        if (decode((char*)appConfiguration["secret_key"].asCString(), jwt, ctx, &result) == 0) {
            ngx_link_func_log(info, ctx, "result %.*s\n",(int) strlen(result.c_str()), result.c_str());
            // always allocate memory, don't need to free as framework will handle the freeing
            char* rs = (char*) ngx_link_func_palloc(ctx, result.length());
            strncpy(rs, result.c_str(), result.length());
            ngx_link_func_add_header_out(ctx, "payload", sizeof("payload") - 1, rs, result.length());

            ngx_link_func_write_resp(
                ctx,
                200,
                NULL,
                ngx_link_func_content_type_plaintext,
                "OK",
                sizeof("OK") - 1
            );
        } else {
            ngx_link_func_log(info, ctx, "Web Token Authenthication Failed");

            ngx_link_func_write_resp(
                ctx,
                403,
                "403 Authenthication Failed",
                "text/plain",
                "",
                0
            );
        }
    } else {
        ngx_link_func_write_resp(
            ctx,
            403,
            "403 Illegal Arguments",
            "text/plain",
            "",
            0
        );
    }
}

// Encode is not in use in this project
// static char* encode(char* secret) {
//     char* jwt;
//     size_t jwt_length;

//     struct l8w8jwt_claim header_claims[] =
//     {
//         {
//             .key = "kid",
//             .key_length = 3,
//             .value = "some-key-id-here-012345",
//             .value_length = strlen("some-key-id-here-012345"),
//             .type = L8W8JWT_CLAIM_TYPE_STRING
//         }
//     };

//     struct l8w8jwt_claim payload_claims[] =
//     {
//         {
//             .key = "ctx",
//             .key_length = 3,
//             .value = "Unforseen Consequences",
//             .value_length = strlen("Unforseen Consequences"),
//             .type = L8W8JWT_CLAIM_TYPE_STRING
//         },
//         {
//             .key = "age",
//             .key_length = 3,
//             .value = "27",
//             .value_length = strlen("27"),
//             .type = L8W8JWT_CLAIM_TYPE_INTEGER
//         },
//         {
//             .key = "size",
//             .key_length = strlen("size"),
//             .value = "1.85",
//             .value_length = strlen("1.85"),
//             .type = L8W8JWT_CLAIM_TYPE_NUMBER
//         },
//         {
//             .key = "alive",
//             .key_length = strlen("alive"),
//             .value = "true",
//             .value_length = strlen("true"),
//             .type = L8W8JWT_CLAIM_TYPE_BOOLEAN
//         },
//         {
//             .key = "nulltest",
//             .key_length = strlen("nulltest"),
//             .value = "null",
//             .value_length = strlen("null"),
//             .type = L8W8JWT_CLAIM_TYPE_NULL
//         }
//     };

//     struct l8w8jwt_encoding_params params;
//     l8w8jwt_encoding_params_init(&params);

//     params.alg = L8W8JWT_ALG_HS256;

//     params.sub = appConfiguration["sub"].asCString();
//     params.sub_length = appConfiguration["iss"].getCStringLength();

//     params.iss = appConfiguration["iss"].asCString();
//     params.iss_length = appConfiguration["iss"].getCStringLength();

//     params.aud = "Administrator";
//     params.aud_length = strlen("Administrator");

//     params.iat = time(NULL);
//     params.exp = time(NULL) + 600; // Set to expire after 10 minutes (600 seconds).

//     params.additional_header_claims = header_claims;
//     params.additional_header_claims_count = sizeof(header_claims) / sizeof(struct l8w8jwt_claim);

//     params.additional_payload_claims = payload_claims;
//     params.additional_payload_claims_count = sizeof(payload_claims) / sizeof(struct l8w8jwt_claim);

//     params.secret_key = (unsigned char*)secret;
//     params.secret_key_length = strlen(secret);

//     params.out = &jwt;
//     params.out_length = &jwt_length;

//     int r = l8w8jwt_encode(&params);
//     printf("\nl8w8jwt_encode_hs256 function returned %s (code %d).\n\nCreated token: \n%s\n", r == L8W8JWT_SUCCESS ? "successfully" : "", r, jwt);

//     // free(jwt);
//     return jwt;
// }

void returnPayload(ngx_link_func_ctx_t* ctx) {
    // ngx_link_func_log(info, ctx, "payload is : %.*s", ctx->req_body_len, (char*)ctx->req_body);

    ngx_link_func_write_resp(
        ctx,
        200,
        "200 OK",
        ngx_link_func_content_type_json,
        (char*) ctx->req_body,
        ctx->req_body_len
    );
}


void
ngx_link_func_init_cycle(ngx_link_func_cycle_t* cyc) {

    Json::Reader reader;

    do {
        std::ifstream file("/etc/nginx/apps/app_config.json");
        if (!reader.parse(file, appConfiguration, false)) {
            ngx_link_func_cyc_log(info, cyc, "Failed to parse configuration\n %s", reader.getFormattedErrorMessages().c_str());
        } else {
            // char .*secret = appConfiguration["secret_key"].asCString();
        }
    } while (0);


}


void
ngx_link_func_exit_cycle(ngx_link_func_cycle_t* cyc) {
    ngx_link_func_cyc_log_info(cyc, "Service is shutting down...........................................");

}

#ifdef __cplusplus
}
#endif