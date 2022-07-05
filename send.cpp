#include <iostream>
#include "cstdlib"
#include "string"
#include "map"
#include "vector"
#include "cstring"
#include "mqtt/client.h"

const std::string SERVER_ADDRESS("tcp://120.76.196.124:50000");
const std::string CLIENT_ID("kaylor_office_PC_send");
const std::string OCU_SEND_TOPIC("kaylor/action");
const int QOS = 0;
const auto TIMEOUT = std::chrono::milliseconds (1);

class user_callback : public virtual mqtt::callback {
  void connection_lost(const std::string &cause) override {
    std::cout << "\nConnection lost" << std::endl;
    if (!cause.empty()) {
      std::cout << "\ncause: " << cause << std::endl;
    }
  }

  void delivery_complete(mqtt::delivery_token_ptr tok) override {
    std::cout << "\n\t[Delivery complete for token: " << (tok ? tok->get_message_id() : -1) << "]" << std::endl;
  }
};

int main() {
  std::cout << "Initializing..." << std::endl;
  mqtt::async_client client(SERVER_ADDRESS, CLIENT_ID);
  user_callback cb;
  client.set_callback(cb);

  mqtt::connect_options connect_options;
  connect_options.set_keep_alive_interval(20);
  connect_options.set_clean_session(true);
  connect_options.set_user_name("kaylor");
  connect_options.set_password("kaylor");
  std::cout << "... OK" << std::endl;
  try {
    std::cout << "\nConnecting..." << std::endl;
    auto conntok = client.connect(connect_options);
    std::cout << "Waiting for the connection.." << std::endl;
    conntok->wait();
    std::cout << "\n...OK" << std::endl;
    std::cout << "\nSending message..." << std::endl;
    auto mqtt_message = mqtt::make_message(OCU_SEND_TOPIC, "this message is from controller");
    mqtt_message->set_qos(QOS);
    client.publish(mqtt_message)->wait_for(TIMEOUT);
//    for (int i = 0; i <10; ++i) {
//      char tmp[128];
//      sprintf(tmp, "value of i is %d", i);
//      mqtt_message = mqtt::make_message(OCU_SEND_TOPIC, tmp);
//      client.publish(mqtt_message)->wait_for(TIMEOUT);
//    }
//    client.publish(mqtt_message);
    std::cout << "\n...OK" << std::endl;
    std::cout << "\nDisconnecting..." << std::endl;
    client.disconnect();
    std::cout << "\n...OK"<< std::endl;
  }catch (const mqtt::persistence_exception& exc) {
    std::cerr << "Persistence Error: " << exc.what() << " ["
         << exc.get_reason_code() << "]" << std::endl;
    return 1;
  }
  catch (const mqtt::exception& exc) {
    std::cerr << "Error: " << exc.what() << " ["
         << exc.get_reason_code() << "]" << std::endl;
    return 1;
  }


  return 0;
}
