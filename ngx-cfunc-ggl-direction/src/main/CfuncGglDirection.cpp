
#include <backcurl/BackCurl.h>
#include <json/json.h>

#include <thread>
#include <fstream>

#ifdef __cplusplus
extern "C" {
#endif
#include <sys/stat.h>
#include <fcntl.h>

#include <ngx_http_c_func_module.h>


extern Json::Value appConfiguration;
Json::Value appConfiguration;


static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}


void getHeartBeat(ngx_http_c_func_ctx_t* ctx) {
    ngx_http_c_func_write_resp(
        ctx,
        200,
        NULL,
        ngx_http_c_func_content_type_plaintext,
        "OK"
    );

}


void getDelay(ngx_http_c_func_ctx_t* ctx) {
    ngx_http_c_func_write_resp(
        ctx,
        200,
        NULL,
        ngx_http_c_func_content_type_plaintext,
        "OK"
    );
}

void getRoute(ngx_http_c_func_ctx_t* ctx) {
    if (ctx->req_args) {
        char *slat = (char*) ngx_http_c_func_get_query_param(ctx, "startLat");
        char *slng = (char*) ngx_http_c_func_get_query_param(ctx, "startLng");
        char *elat = (char*) ngx_http_c_func_get_query_param(ctx, "endLat");
        char *elng = (char*) ngx_http_c_func_get_query_param(ctx, "endLng");


        if (slat && slng && elat && elng) {
            ngx_http_c_func_log(info, ctx, "Args are %s, %s, %s, %s", slat, slng, elat, elng);

            // double startLat = atof(slat);
            // double startlng = atof(slng);
            // double endLat = atof(elat);
            // double endLng = atof(elng);

            std::string url = appConfiguration["domainUrlPath"].asString();
            url.append("origin=");
            url.append(slat);
            url.append(",");
            url.append(slng);
            url.append("&destination=");
            url.append(elat);
            url.append(",");
            url.append(elng);

            bcl::execute<std::string>([ & ](bcl::Request * req) {
                // req->headers.emplace_back("Content-Type", "application/x-www-form-urlencoded");
                // req->headers.emplace_back("Connection", "Keep-Alive");
                // req->headers.emplace_back("Charset", "UTF-8");

                char *fullUrl = (char*)url.c_str();

                ngx_http_c_func_log(info, ctx, "fullUrl = %s", fullUrl);

                bcl::setOpts(req, CURLOPT_URL , fullUrl,
                             // CURLOPT_USERAGENT, "Mozilla/5.0",
                             CURLOPT_FOLLOWLOCATION, 1L,
                             CURLOPT_SSL_VERIFYPEER, 0L,
                             CURLOPT_WRITEFUNCTION, WriteCallback,
                             CURLOPT_WRITEDATA, req->dataPtr
                            );
            }, [&](bcl::Response * resp) {
                ngx_http_c_func_log(info, ctx, "Status Code= %ld", resp->code);

                char* resultStr = (char*)resp->getBody<std::string>()->c_str();

                // ngx_http_c_func_log(info, ctx,"Result = %s", resultStr);

                Json::Value rs;
                Json::Reader reader;
                bool parsingSuccessful = reader.parse( resultStr , rs );     //parse process
                if ( !parsingSuccessful )
                {

                    ngx_http_c_func_log(err, ctx, "Failed to parse: %s", reader.getFormattedErrorMessages().c_str());
                    ngx_http_c_func_write_resp(
                        ctx,
                        200,
                        NULL,
                        ngx_http_c_func_content_type_json,
                        "{'status':0}"
                    );
                }

                if ( ! rs["routes"][0].empty() ) {
                    Json::Value legs = rs["routes"][0]["legs"];
                    if (!legs.empty()) {
                        Json::Value leg = rs["routes"][0]["legs"][0];

                        Json::Value shrinkResult;
                        shrinkResult["distance"] = leg["distance"]["value"];
                        shrinkResult["ett"] = leg["duration"]["value"];
                        shrinkResult["startLat"] = leg["start_location"]["lat"];
                        shrinkResult["startLng"] = leg["start_location"]["lng"];
                        shrinkResult["endLat"] = leg["end_location"]["lat"];
                        shrinkResult["endLng"] = leg["end_location"]["lng"];

                        Json::FastWriter fastWriter;
                        std::string output = fastWriter.write(shrinkResult);
                        ngx_http_c_func_write_resp(
                            ctx,
                            200,
                            NULL,
                            ngx_http_c_func_content_type_json,
                            output.c_str()
                        );
                    }
                }
            });

        } else {
            ngx_http_c_func_write_resp(
                ctx,
                200,
                NULL,
                ngx_http_c_func_content_type_json,
                "{'status':0}"
            );
        }

    } else {
        ngx_http_c_func_write_resp(
            ctx,
            200,
            NULL,
            ngx_http_c_func_content_type_json,
            "{'status':0}"
        );
    }
}


void ngx_http_c_func_init(ngx_http_c_func_ctx_t *ctx) {
    bcl::init();

    Json::Reader reader;

    do {
        std::ifstream file("/etc/nginx/app_config.json");
        if (!reader.parse(file, appConfiguration, false)) {
            ngx_http_c_func_log(info, ctx, "Failed to parse configuration\n %s", reader.getFormattedErrorMessages().c_str());
        }
    } while (0);
}


void ngx_http_c_func_exit(ngx_http_c_func_ctx_t *ctx) {
    ngx_http_c_func_log_info(ctx, "Service is shutting down...........................................");

    bcl::cleanUp();
}

#ifdef __cplusplus
}
#endif