/*
Author: Zilong Fan
Class: ECE6122
Last Date Modified: 11/21/2023
Description:
 TCP Client
*/ 

// Use the SFML examples

#include <SFML/Network.hpp>
#include <iostream>
#include <list>
#include <mutex>
#include <thread>
#include <atomic>
#include <string>
#include <cstring>
#include <sstream>

struct tcpMessage
{
    unsigned char nVersion;
    unsigned char nType;
    unsigned short nMsgLen;
    char chMsg[1000];
}; 

sf::Packet& operator <<(sf::Packet& packet, const tcpMessage& character)
{
    return packet << character.nVersion << character.nType << character.nMsgLen << character.chMsg;
}

sf::Packet& operator >>(sf::Packet& packet, tcpMessage& character)
{
    return packet >> character.nVersion >> character.nType >> character.nMsgLen >> character.chMsg;
}

// Deal the message in another thread
void message_thread(sf::TcpSocket& socket, std::atomic<bool>& running) {
    sf::Packet packet;
    while (running) {
        socket.setBlocking(false);

        if (socket.receive(packet) == sf::Socket::Done) 
        {
            std::cout << "  Received Message Report:" << std::endl;
            tcpMessage receivedMsg;
            packet >> receivedMsg;
            std::cout << "  Received Msg Type: " << (int)receivedMsg.nType << "; Msg: " << receivedMsg.chMsg << std::endl;
            std::cout << "  Please re input the command " ;

            // std::cout << "  Received Msg from IP: " <<  << std::endl;
            // std::cout << "  Received Msg fromt port: " <<  << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {

    //bool is_exit;

    if (argc != 3) {
        std::cerr << "Standard Usage should be : " << argv[0] << " <ip> <port>" << std::endl;
        return 1;
    }

    sf::IpAddress ip(argv[1]);

    // only stoi
    unsigned short port = std::stoi(argv[2]);

    sf::TcpSocket socket;

    if (socket.connect(ip, port) != sf::Socket::Done) {
        std::cerr << "Unable to connect to server!" << std::endl;
        return 1;
    }
    else
    {
        std::cout << "Connect to server successfully! " << std::endl;
    }

    std::string input;
    std::string command;
    tcpMessage message;
    std::string message_input;
    int num_input;

    std::atomic<bool> running_Signal(true);
    std::thread receiveThread(message_thread, std::ref(socket), std::ref(running_Signal));
    receiveThread.detach();

    while (true) 
    {
        std::cout << std::endl;
        std::cout << "Please enter command: ";
        std::getline(std::cin, input);
        std::stringstream sS(input);
        sS >> command >> num_input >> message_input;

        if (command == "q") 
        {
            std::cout << "  Execute command q successfully." << std::endl;
            running_Signal = false;
            break;
        } 
        else if (command[0] == 'v') 
        {
            message.nVersion = num_input;
            std::cout << "  Execute command v successfully." << std::endl;
        } 
        else if (command[0] == 't') 
        {
            message.nType = num_input;

            std::string msg = message_input;

            //message.chMsg = msg; // Wrong???

            strncpy(message.chMsg, msg.c_str(), sizeof(message.chMsg));

            message.nMsgLen = msg.length();

            sf::Packet packet;
            packet << message;
            //packet.append(&message, sizeof(message));
            socket.send(packet);
            std::cout << "  Execute command t successfully." << std::endl;
        }
    }

    // Skip Error
    if (receiveThread.joinable()) {
        receiveThread.join(); 
    }

    socket.disconnect();
    return 0;
}
