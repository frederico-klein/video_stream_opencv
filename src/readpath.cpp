#include <fts.h> // i should have done it all using just boost, but I was having trouble with the iterators and found this thing, so I will use both until i can do it all using just boost.
#include <string.h>
#include <cstdio>
#include <errno.h>    //for error handling
#include <iostream>
#include <boost/filesystem.hpp>
#include <ros/ros.h>
//#include <std_srvs/String.h>
//#include <std_srvs/Empty.h>
#include "video_stream_opencv/actvid.h"


FTS* tree=NULL;
FTSENT* node=NULL;

//bool readnext(std_srvs::Empty::Request &req, std_srvs::Empty::Response &res) {
bool readnext(video_stream_opencv::actvid::Request &req, video_stream_opencv::actvid::Response &res) {

//put everything in here, create topics to be published underneath and update them in this callback
  ROS_INFO("readnext service was called. ");
  bool outputtedfile = false;
  while (node = fts_read(tree)) {
      std::string mypath = node->fts_path;
      ROS_INFO("relative path (fts_path): %s",mypath.c_str());
      boost::filesystem::path currentfile = mypath+ (node->fts_name);
      ROS_INFO("full path of currentfile (boost): %s",currentfile.string().c_str());
      std::string extension = boost::filesystem::extension(currentfile);
      ROS_INFO("extension of currentfile (boost): %s",extension.c_str());
      //printf("the hell %s\n", extension.c_str());

      if (node->fts_level > 0 && node->fts_name[0] == '.' )//(extension.compare(".avi")!= 0)) // || !(extension == ".avi"))
          fts_set(tree, node, FTS_SKIP);
      else if (node->fts_info & FTS_F) {
          if(extension.compare(".avi")==0 || extension.compare(".mp4")==0){
            res.Action = currentfile.parent_path().filename().string();
            printf("%d is the same result\n", extension.compare(".avi"));
            printf("Got video named %s\n at depth %d,\n "
              "accessible via %s from the current directory \n"
              "or via %s from the original starting directory\n\n",
              node->fts_name, node->fts_level,
              node->fts_accpath, node->fts_path);
            outputtedfile = true;
            res.File = node->fts_accpath;
            res.ActionDefined = true;

          /* if fts_open is not given FTS_NOCHDIR,
           * fts may change the program's current working directory */
           }
      }
      if (outputtedfile){
        return true; //strange syntax, see if it works
      }
  }
  ROS_INFO_STREAM("No more files found. I should terminate, don't you think?");
  return true;
}


int main(int argc, char **argv) {
  //TODO: initialize this as a ros thingy. get parameters from launch file,
    ros::init(argc, argv, "readpath_service_srv");
    //ros::NodeHandle nh;
    ros::NodeHandle _nh("~"); // to get the private params
    std::string basepath;
    _nh.getParam("basepath", basepath);
    ros::ServiceServer service = _nh.advertiseService("read_next", readnext);
    ROS_INFO("got so far ");
    //char **paths = (char**)(basepath.c_str()); SIGSEVG much!
    char const *paths[] = { basepath.c_str(), NULL };
    tree = fts_open((char**)paths, FTS_NOCHDIR, 0);
    if (!tree) {
        perror("fts_open");
        ROS_ERROR("fts_open failed ");
        return 1;

    }

    ROS_INFO("readpath service ready to request a path");
    ros::spin();
    if (errno) {
        perror("fts_read");
                ROS_ERROR("fts_readfailed ");
        return 1;
    }

    if (fts_close(tree)) {
        perror("fts_close");
                ROS_ERROR("fts_close failed ");
        return 1;
        }
    return 0;
    //const char *dot[] = {".", 0};
    //char **paths = argc > 1 ? argv + 1 : (char**) dot;



    }
