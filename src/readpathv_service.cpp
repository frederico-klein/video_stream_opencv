#include <fts.h> // i should have done it all using just boost, but I was having trouble with the iterators and found this thing, so I will use both until i can do it all using just boost.
#include <string.h>
#include <cstdio>
#include <errno.h>    //for error handling
#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/filesystem.hpp>
#include <ros/ros.h>
#include <std_msgs/String.h>
//#include <std_srvs/Empty.h>
#include "video_stream_opencv/split.h"
#include "video_stream_opencv/actvid.h"


//FTS* tree=NULL;
//FTSENT* node=NULL;

std::string curract, currfile;

// so as a simpler fix I will make a publisher out of this guy as well. the best idea here is to make it an additional service call or to intercept the service call? with a flexbe, i think
std_msgs::String label_y_msg;
ros::Publisher labels;

std_msgs::String done_msg;
ros::Publisher done;

class MovieFile{
public:
  std::string File, Action, filename;
  bool ActionDefined;



};

std::vector<MovieFile> allMovies;

    std::string basepath, splitpath;
//bool readnext(std_srvs::Empty::Request &req, std_srvs::Empty::Response &res) {
bool readnext(video_stream_opencv::actvid::Request &req, video_stream_opencv::actvid::Response &res) {

//put everything in here, create topics to be published underneath and update them in this callback
  ROS_INFO("readnext service was called. ");
  bool outputtedfile = false;
  if (allMovies.size()>0) {
            res.Action = allMovies[0].Action;
            outputtedfile = true;
            res.File = allMovies[0].File;
            res.ActionDefined = allMovies[0].ActionDefined;
            ROS_INFO("Action: %s\nFile:%s\n",res.Action.c_str(),res.File.c_str());
            currfile = res.File;
            curract = res.Action;
            label_y_msg.data = curract;
            ROS_INFO("y topic message:%s",label_y_msg.data.c_str());
            labels.publish(label_y_msg);
            allMovies.erase(allMovies.begin());
          /* if fts_open is not given FTS_NOCHDIR,
           * fts may change the program's current working directory */
           done_msg.data = "0";
           ROS_INFO("done topic message:%s",done_msg.data.c_str());
           done.publish(done_msg);
           return true; //strange syntax, see if it works

      } else
      {
        done_msg.data = "1";
        ROS_INFO("done topic message:%s. I'm done btw",done_msg.data.c_str());
        done.publish(done_msg);

      }
  ROS_INFO_STREAM("No more files found. I should terminate, don't you think?");
  return true;
}

bool curritem(video_stream_opencv::actvid::Request &req, video_stream_opencv::actvid::Response &res) {
  ROS_INFO("curritem service was called. ");
  res.File = currfile;
  res.Action = curract;
  res.ActionDefined = true;
}


bool readsplit(video_stream_opencv::split::Request &req, video_stream_opencv::split::Response &res) {
  FTS* tree=NULL;
  FTSENT* node=NULL;
  //need to clear the current allMovies or things look weird
  allMovies.clear();
  char const *paths[] = { basepath.c_str(), NULL };
  tree = fts_open((char**)paths, FTS_NOCHDIR, 0);
  if (!tree) {
      perror("fts_open");
      ROS_ERROR("fts_open failed ");
      return 1;

  }


  while (node = fts_read(tree)) {
      MovieFile thisMovie;
      std::string mypath = node->fts_path;
      ROS_DEBUG("relative path (fts_path): %s",mypath.c_str());
      boost::filesystem::path currentfile = mypath;//+ (node->fts_name);
      ROS_DEBUG("full path of currentfile (boost): %s",currentfile.string().c_str());
      std::string extension = boost::filesystem::extension(currentfile);
      ROS_DEBUG("extension of currentfile (boost): %s",extension.c_str());
      //printf("the hell %s\n", extension.c_str());

      if (node->fts_level > 0 && node->fts_name[0] == '.' )//(extension.compare(".avi")!= 0)) // || !(extension == ".avi"))
          fts_set(tree, node, FTS_SKIP);
      else if (node->fts_info & FTS_F) {
          if(extension.compare(".avi")==0 || extension.compare(".mp4")==0){
            thisMovie.Action = currentfile.parent_path().filename().string();
            ROS_DEBUG("%d is the same result\n", extension.compare(".avi"));
            ROS_DEBUG("Got video named %s\n at depth %d,\n "
              "accessible via %s from the current directory \n"
              "or via %s from the original starting directory\n\n",
              node->fts_name, node->fts_level,
              node->fts_accpath, node->fts_path);
            thisMovie.File = node->fts_accpath;
            thisMovie.filename = currentfile.filename().string();
            ROS_DEBUG("thisMovie.filename: %s",thisMovie.filename.c_str());
            thisMovie.ActionDefined = true;
            /* if fts_open is not given FTS_NOCHDIR,
           * fts may change the program's current working directory */
           allMovies.push_back(thisMovie);
           }
      }

  }

  printf("%lu\n", allMovies.size());
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


    //read splits region:
    boost::filesystem::path splitdir = splitpath;//"/home/frederico/Documents/catkin_backup/src/video_stream_opencv/data/hmdb51_7030splits";
    //std::string splitdelim = "split";
    std::string s;
    //int split = 1;
    std::ifstream myfile;
    int splitnum;
    std::string line;
    ROS_INFO("Selecting split %d",req.Split);
    ROS_DEBUG("splitdir: %s",splitdir.c_str());
    if (boost::filesystem::is_directory(splitdir))
    {
      ROS_INFO("Opening directory to verify splits: %s",splitdir.c_str());
      for (auto& entry: boost::make_iterator_range(boost::filesystem::directory_iterator(splitdir),{}))
      {
        s = entry.path().filename().string().c_str();
        ROS_DEBUG("Verifying split file named: %s",s.c_str());
        splitnum = std::stoi(s.substr(s.find("split")+5,1).c_str());
        ROS_DEBUG("Split: %d",splitnum);
        if (splitnum==req.Split)// && !s.compare("fencing_test_split1.txt"))
        {
          ROS_DEBUG("%d is the correct split.",splitnum);
          myfile.open(entry.path().string());

          while(std::getline(myfile,line))
          {
            std::istringstream iss(line);
            std::string movieline;
            int test_train_nothing;
            iss >> movieline >> test_train_nothing;
            if (test_train_nothing==2)
            {
                ROS_DEBUG("%s is included for testing! doing nothing.",movieline.c_str());
            }
            else
            {
              ROS_DEBUG("%s is member of %d, not used for testing",movieline.c_str(),test_train_nothing);
              for (int i=0; i<allMovies.size();i++)
              {
                ROS_DEBUG("%s %s",allMovies[i].filename.c_str(), movieline.c_str());
                if (movieline.compare(allMovies[i].filename) == 0)
                {

                  ROS_DEBUG("movie found, will be removed from allMovies.");
                  //if it finds the video, since it will only be here once, I can break from the for loop
                  allMovies.erase(allMovies.begin()+i);
                  break;
                } else
                {
                  //ROS_DEBUG("%s %s",allMovies[i].filename.c_str(), movieline.c_str());
                }
              }
            }

          }
          myfile.close();
        }
        else
        {
          ROS_DEBUG("%d does not match selected split, doing nothing.",splitnum);
        }
        //if blablablah...
      }
    }
    printf("%lu\n", allMovies.size());


    ROS_INFO("readpath service ready to request a path");
    return true;
}

int main(int argc, char **argv) {

    ros::init(argc, argv, "readpath_service_srv");
    //ros::NodeHandle nh;
    ros::NodeHandle _nh("~"); // to get the private params
    _nh.getParam("basepath", basepath);
    _nh.getParam("splitdir", splitpath);
    ros::ServiceServer service_rn = _nh.advertiseService("read_next", readnext);
    ROS_DEBUG("instantiated service read_next ");
    ros::ServiceServer service_rs = _nh.advertiseService("read_split", readsplit);
    ROS_DEBUG("instantiated service read_split ");

    labels = _nh.advertise<std_msgs::String>("y",100);
    done = _nh.advertise<std_msgs::String>("done",100);

    ROS_INFO("defined publisher for y and done.");
    //char **paths = (char**)(basepath.c_str()); SIGSEVG much!
    ros::spin();

    return 0;
}



    //const char *dot[] = {".", 0};
    //char **paths = argc > 1 ? argv + 1 : (char**) dot;
