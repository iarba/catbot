#include "macro.hpp"

#include <cstring>
#include <cstdlib>
#include <thread>

int http_main(int argc, char **argv);
int dpp_main(int argc, char **argv);

bool pmode = false;

int main(int argc, char **argv)
{
  for(int i = 0; i < argc; i++)
  {
    if(strcmp(argv[i], "prod") == 0)
    {
      pmode = true;
    }
  }
  WARN("starting in %s mode\n", pmode ? "production" : "dev");
  std::thread dpp_thr(dpp_main, argc, argv);
  return http_main(argc, argv);
}
