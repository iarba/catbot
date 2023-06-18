#define CPPHTTPLIB_OPENSSL_SUPPORT
#define HTTPS_SERVER

#include "httplib.h"

#include <openssl/md5.h>

typedef enum
{
  catgirl_CHOCOLA = 0,
  catgirl_VANILLA,
  catgirl_MAX
} catgirl_t;

typedef struct
{
  std::string name;
  std::string data;
  std::string content_type;
} catgirl_payload_t;

catgirl_t next_selection(catgirl_t current)
{
  return (catgirl_t)((current + 1) % catgirl_MAX);
}

// initialise rotating buffer
std::mutex selection_lock;
catgirl_t selection = catgirl_CHOCOLA;

// create a database of catgirls
catgirl_payload_t catgirls[catgirl_MAX];

void process_catgirl(catgirl_payload_t *cg, httplib::Result &res)
{
  printf("INFO: processing %s\n", cg->name.c_str());
  if(!res)
  {
    printf("ERROR: failed to get %s http error\n", cg->name.c_str());
    return;
  }
  if(res->status != 200)
  {
    printf("ERROR: failed to get %s status %d\n", cg->name.c_str(), res->status);
    return;
  }
  cg->data = res->body;
  try
  {
    cg->content_type = res->get_header_value("Content-Type");
  }
  catch(std::exception &_e)
  {
    printf("WARN: failed to automatically detect content type for %s, using default\n", cg->name.c_str());
    cg->content_type = "image/gif";
  }
}

void load_catgirls()
{
  catgirl_t current_catgirl;
  // first, fetch chocola
  {
    current_catgirl = catgirl_CHOCOLA;
    catgirls[current_catgirl].name = "Chocola";
    httplib::Client cli("https://i.pinimg.com");
    auto res = cli.Get("/originals/ba/12/b5/ba12b5f2160d7b4ab755e88d1121303f.gif");
    process_catgirl(catgirls + current_catgirl, res);
  }
  // next, fetch vanilla
  {
    current_catgirl = catgirl_VANILLA;
    catgirls[current_catgirl].name = "Vanilla";
    httplib::Client cli("https://i.imgur.com");
    auto res = cli.Get("/0h3vuUT.gif");
    process_catgirl(catgirls + current_catgirl, res);
  }
  // nothing left to fetch
}

int main(int argc, char **argv)
{
  bool test_mode = true;
  uint16_t port;
  for(int i = 0; i < argc; i++)
  {
    if(strcmp(argv[i], "prod") == 0)
    {
      test_mode = false;
    }
  }
  if(test_mode)
  {
    port = 8080;
    printf("deploying for testing on port %d\n", port);
  }
  else
  {
    port = 443;
    printf("deploying for production on port %d\n", port);
  }
#ifdef HTTPS_SERVER
  // maybe replace these values with your certificate locations
  httplib::SSLServer svr("/opt/crt/iarba.xyz/cert.pem", "/opt/crt/iarba.xyz/key.pem");
#else // HTTPS_SERVER
  httplib::Server svr;
#endif // HTTPS_SERVER
  printf("INFO: loading catgirls\n");
  load_catgirls();
  printf("INFO: creating endpoint\n");
  svr.Get("/welcome.gif", [](const httplib::Request &req, httplib::Response &res) {
    // select a catgirl to serve
    catgirl_t current_selection;
    if (req.has_param("ts")) {
      // pick a catgirl deterministically, based on the timestamp hash
      auto val = req.get_param_value("ts");
      uint8_t hash_res[MD5_DIGEST_LENGTH], hash_fold = 0;
      MD5((uint8_t *)val.c_str(), val.size(), hash_res);
      for(int i = 0; i < MD5_DIGEST_LENGTH; i++)
      {
        hash_fold = hash_fold ^ hash_res[i];
      }
      current_selection = (catgirl_t)(hash_fold % catgirl_MAX);
      printf("INFO: deterministic selection, %d(%s)\n", current_selection, catgirls[current_selection].name.c_str());
    }
    else
    {
      // cycle a catgirl at random
      selection_lock.lock();
      current_selection = selection;
      selection = next_selection(current_selection);
      selection_lock.unlock();
      printf("INFO: cyclic selection, %d(%s)\n", current_selection, catgirls[current_selection].name.c_str());
    }
    // serve said catgirl from our data
    res.set_content(catgirls[current_selection].data, catgirls[current_selection].content_type);
    res.set_header("Cache-Control", "no-cache");
    res.set_header("Catgirl-Name", catgirls[current_selection].name);
    res.status = 200;
  });

  printf("INFO: attaching logger\n");
  svr.set_logger([](const auto& req, const auto& res) {
    printf("INFO: Got request %s %s status %d\n", req.method.c_str(), req.path.c_str(), res.status);
  });

  printf("INFO: listening ipv4 port %u\n", port);
  svr.listen("0.0.0.0", port);
  return 0;
}