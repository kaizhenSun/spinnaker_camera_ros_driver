#ifndef NODELET_ROSBAG_H
#define NODELET_ROSBAG_H

#include <boost/thread/mutex.hpp>
#include <nodelet/nodelet.h>
#include <rosbag/bag.h>
#include <topic_tools/shape_shifter.h>
#include <actionlib/server/simple_action_server.h>
#include <nodelet_rosbag/SubscribeAction.h>
#include <nodelet_rosbag/StartAction.h>
#include <nodelet_rosbag/StopAction.h>
#include <nodelet_rosbag/RecordAction.h>
#include <boost/utility/in_place_factory.hpp>

namespace nodelet_rosbag
{

class NodeRosbagImpl {
public:
  NodeRosbagImpl(ros::NodeHandle *nh, ros::NodeHandle *private_nh) :
    nh_(*nh), private_nh_(*private_nh),
    record_actionserver_(nh_, "record", false),
    start_actionserver_(nh_, "start", false),
    stop_actionserver_(nh_, "stop", false),
    recording_(false) {
    private_nh_.getParam("rosbag_path", rosbag_path_);
    private_nh_.getParam("rosbag_record_topics", rosbag_record_topics_);

    record_actionserver_.registerGoalCallback(
      boost::bind(&NodeRosbagImpl::mode_callback, this));
    record_actionserver_.start();

    start_actionserver_.registerGoalCallback(
      boost::bind(&NodeRosbagImpl::start_callback, this));
    start_actionserver_.start();

    stop_actionserver_.registerGoalCallback(
      boost::bind(&NodeRosbagImpl::stop_callback, this));
    stop_actionserver_.start();
}

private:
  ros::NodeHandle &nh_;
  ros::NodeHandle &private_nh_;

  typedef actionlib::SimpleActionServer<nodelet_rosbag::RecordAction> record_actionserver_type;

  record_actionserver_type record_actionserver_;

  typedef actionlib::SimpleActionServer<nodelet_rosbag::StartAction> start_actionserver_type;

  start_actionserver_type start_actionserver_;

  typedef actionlib::SimpleActionServer<nodelet_rosbag::StopAction> stop_actionserver_type;

  stop_actionserver_type stop_actionserver_;

  // Write messages coming to this callback to a Bag file
  void record_callback(const ros::MessageEvent<topic_tools::ShapeShifter const> & event);

  // Switch between recording and playback
  void mode_callback();

  // Start callback
  void start_callback();

  // Stop callback
  void stop_callback();

  std::map<std::string, ros::Publisher> playback_publishers_;
  std::vector<ros::Subscriber> record_subscribers_;
  rosbag::Bag bag_;
  std::string rosbag_path_;
  std::vector<std::string> rosbag_record_topics_;

  bool recording_;
  boost::mutex rosbag_mode_mtx_;
  boost::mutex rosbag_bag_mtx_;
};

class NodeletRosbag : public nodelet::Nodelet
{
public:
  virtual void onInit() {
    NODELET_DEBUG("Initializing nodelet...");
    node_impl_ = boost::in_place(&getNodeHandle(), &getPrivateNodeHandle());
  }

private:
  boost::optional<NodeRosbagImpl> node_impl_;
};

}

PLUGINLIB_EXPORT_CLASS(nodelet_rosbag::NodeletRosbag, nodelet::Nodelet);

#endif // NODELET_ROSBAG_H
