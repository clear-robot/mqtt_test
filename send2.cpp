//
// Created by kaylor on 22-7-5.
//
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cctype>
#include <thread>
#include <chrono>
#include "mqtt/async_client.h"

const std::string SERVER_ADDRESS("tcp://120.76.196.124:50000");
const std::string CLIENT_ID("kaylor_office_PC_send2");
const std::string S_SEND_TOPIC("kaylor/action");
const std::string S_REV_TOPIC("kaylor/feedback");
const int QOS = 0;
const int N_RETRY_ATTEMPTS = 5;
const auto TIMEOUT = std::chrono::milliseconds (1);

class action_listener : public virtual mqtt::iaction_listener {
  std::string name_;

  void on_failure(const mqtt::token &tok) override {
    std::cout << name_ << " failure";
    if (tok.get_message_id() != 0)
      std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
    std::cout << std::endl;
  }

  void on_success(const mqtt::token &tok) override {
    std::cout << name_ << " success";
    if (tok.get_message_id() != 0)
      std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
    auto top = tok.get_topics();
    if (top && !top->empty())
      std::cout << "\ttoken topic: '" << (*top)[0] << "', ..." << std::endl;
    std::cout << std::endl;
  }

 public:
  action_listener(const std::string &name) : name_(name) {}
};

class callback : public virtual mqtt::callback,
                 public virtual mqtt::iaction_listener {
  int nretry_;
  mqtt::async_client &cli_;
  mqtt::connect_options &connOpts_;
  action_listener subListener_;

  void reconnect() {
    std::this_thread::sleep_for(std::chrono::milliseconds(2500));
    try {
      cli_.connect(connOpts_, nullptr, *this);
    }
    catch (const mqtt::exception &exc) {
      std::cerr << "Error: " << exc.what() << std::endl;
      exit(1);
    }
  }

  void on_failure(const mqtt::token &tok) override {
    std::cout << "Connection attempt failed" << std::endl;
    if (++nretry_ > N_RETRY_ATTEMPTS)
      exit(1);
    reconnect();
  }

  void on_success(const mqtt::token &tok) override {}

  void connected(const std::string &cause) override {
    std::cout << "\nConnection success" << std::endl;
    std::cout << "\nSubscribing to topic '" << S_REV_TOPIC << "'\n"
              << "\tfor client " << CLIENT_ID
              << " using QoS" << QOS << "\n"
              << "\nPress Q<Enter> to quit\n" << std::endl;

    cli_.subscribe(S_REV_TOPIC, QOS, nullptr, subListener_);
  }

  void connection_lost(const std::string &cause) override {
    std::cout << "\nConnection lost" << std::endl;
    if (!cause.empty())
      std::cout << "\tcause: " << cause << std::endl;

    std::cout << "Reconnecting..." << std::endl;
    nretry_ = 0;
    reconnect();
  }

  void message_arrived(mqtt::const_message_ptr msg) override {
    std::cout << "Message arrived" << std::endl;
    std::cout << "\ttopic: '" << msg->get_topic() << "'" << std::endl;
    std::cout << "\tpayload: '" << msg->to_string() << "'\n" << std::endl;
  }

  void delivery_complete(mqtt::delivery_token_ptr token) override {}

 public:
  callback(mqtt::async_client &cli, mqtt::connect_options &connOpts)
      : nretry_(0), cli_(cli), connOpts_(connOpts), subListener_("Subscription") {}
};

int main(int argc, char *argv[]) {

  mqtt::connect_options connect_options;
  connect_options.set_keep_alive_interval(20);
  connect_options.set_clean_session(true);
  connect_options.set_user_name("kaylor");
  connect_options.set_password("kaylor");

  mqtt::async_client client(SERVER_ADDRESS, CLIENT_ID);

  callback cb(client, connect_options);
  client.set_callback(cb);

  try {
    std::cout << "Connecting to the MQTT server..." << std::flush;
    auto tok = client.connect(connect_options, nullptr, cb);
    tok->wait();
  }
  catch (const mqtt::exception &) {
    std::cerr << "\nERROR: Unable to connect to MQTT server: '"
              << SERVER_ADDRESS << "'" << std::endl;
    return 1;
  }

  auto mqtt_message = mqtt::make_message(S_SEND_TOPIC, "This message is from controller");
  mqtt_message->set_qos(QOS);
  client.publish(mqtt_message)->wait_for(TIMEOUT);

  while (std::tolower(std::cin.get()) != 'q'){}

  try {
    std::cout << "\nDisconnecting from the MQTT server..." << std::flush;
    client.disconnect()->wait();
    std::cout << "OK" << std::endl;
  }
  catch (const mqtt::exception &exc) {
    std::cerr << exc.what() << std::endl;
    return 1;
  }

  return 0;
}