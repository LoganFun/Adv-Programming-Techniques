/*
Author: Zilong Fan
Class: ECE6122
Last Date Modified: 11/21/2023
Description:
 TCP Server
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

// Atomic Value for thread blocking
std::atomic<bool> running_Signal(true);
std::mutex clientsMutex;

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

void processClient(sf::TcpSocket* client, std::list<sf::TcpSocket*>& clients, 
    sf::SocketSelector& selector, std::string& lastMessage) 
{
    while (running_Signal) 
    {
        // Wait and isReady must be together
        if (selector.wait())
        {
            if (selector.isReady(*client)) 
            {
                sf::Packet packet;

                if (client->receive(packet) == sf::Socket::Done) 
                {
                    std::cout<<std::endl;
                    std::cout<<"Sucessfully Received Packet from client."<<std::endl;
                    std::cout<<"    IP Address: "<< client->getRemoteAddress()<<" | Port: "<< client->getRemotePort()<<std::endl;

                    // Process packet
                    // Lock the Function I need 
                    std::lock_guard<std::mutex> guard(clientsMutex);

                    // Initialize the Last Message
                    lastMessage.clear();

                    // Receive the packet
                    tcpMessage Message;
                    packet >> Message;

                    // Int can use the compare with unsigned char !!!

                    if (Message.nVersion == 102)
                    {
                        lastMessage = Message.chMsg;
                        if (Message.nType == 77)
                        {                            
                            // Echo received message to all connected clients except the sender
                            for (auto& otherClient : clients) {
                                if (otherClient != client) {
                                    otherClient->send(packet);
                                }
                            }
                            std::cout<<"Sucessfully Distribute Packet according the nType 77."<<std::endl;
                            std::cout<<std::endl;
                        }
                        else if (Message.nType == 201)
                        {
                            // Reverse it 
                            std::string temp_str = Message.chMsg;
                            std::reverse(temp_str.begin(), temp_str.end());
                            // Prepare new message with reversed content
                            tcpMessage newMessage;
                            newMessage.nVersion = Message.nVersion;
                            newMessage.nType = Message.nType;
                            newMessage.nMsgLen = static_cast<unsigned short>(temp_str.size());
                            std::strcpy(newMessage.chMsg, temp_str.c_str()); // Copy reversed string back into the message

                            sf::Packet newPacket;
                            newPacket << newMessage;

                            // Echo received message to all connected clients except the sender
                            for (auto& otherClient : clients) {
                                if (otherClient == client) {
                                    otherClient->send(newPacket);
                                }
                            }
                            std::cout<<"Sucessfully Distribute Packet according the nType 201."<<std::endl;
                            std::cout<<std::endl;
                        }  
                        else
                        {
                            lastMessage = Message.chMsg;
                            std::cout << "Not 201 and 77 " << std::endl;
                            std::cout<<std::endl;
                        }                      
                    }
                } 
                else 
                {
                    // Client has disconnected
                    // Select remove
                    selector.remove(*client);
                    client->disconnect();

                    //clients Remove use locker
                    clientsMutex.lock();
                    clients.remove(client);
                    clientsMutex.unlock();

                    // delete it memory
                    delete client;
                    break;
                }
            }
        }
    }
}

void buildConnection(sf::TcpListener& listener, std::list<sf::TcpSocket*>& clients, 
    sf::SocketSelector& selector, std::string& lastMessage) 
{
    while (running_Signal) {

        // Wait and isReady must be together
        if (selector.wait()) 
        {
            if (selector.isReady(listener)) 
            {
                // Give memory space
                sf::TcpSocket* client = new sf::TcpSocket();

                if (listener.accept(*client) == sf::Socket::Done) 
                {
                    //Lock the add process
                    clientsMutex.lock();
                    clients.push_back(client);
                    selector.add(*client);
                    clientsMutex.unlock();

                    // Create the thread for new client
                    // Use std::ref to reference the value
                    std::thread clientThread(processClient, std::ref(client), std::ref(clients), std::ref(selector), std::ref(lastMessage));
                    clientThread.detach();
                } 
                else 
                {
                    delete client;
                }
            }
        }
    }
}

void userCommands(
    //sf::TcpListener& listener, , sf::SocketSelector& selector
    std::list<sf::TcpSocket*>& clients, std::string& lastMessage) 
{
    std::string command;

    while (running_Signal) 
    {
        std::cout << "Please enter command: ";
        std::getline(std::cin, command);

        if (command == "msg") 
        {
            std::lock_guard<std::mutex> guard(clientsMutex);
            std::cout << "Last Message: " << lastMessage << std::endl;
        } 
        else if (command == "clients") 
        {
            std::lock_guard<std::mutex> guard(clientsMutex);
            std::cout << "Number of Clients: " << clients.size() << std::endl;
            for (auto* client : clients) 
            {
                std::cout << "    IP Address: " << client->getRemoteAddress() << " | Port: " << client->getRemotePort() << std::endl;
            }
        } 
        else if (command == "exit") 
        {
            running_Signal = false;
            // Create a special message indicating server shutdown
            tcpMessage shutdownMessage;
            shutdownMessage.nVersion = 0; 
            shutdownMessage.nType = 100; 
            std::strcpy(shutdownMessage.chMsg, "Server is shutting down");
            shutdownMessage.nMsgLen = std::strlen(shutdownMessage.chMsg);

            sf::Packet shutdownPacket;
            shutdownPacket << shutdownMessage;

            // Send shutdown message to all clients
            for (auto* client : clients) 
            {
                client->send(shutdownPacket);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    
    if (argc != 2) {
        
        std::cerr << "Standard Usage should be : " << argv[0] << " <port>" << std::endl;

        return 1;
    }

    std::list<sf::TcpSocket*> clients;
    unsigned short port = std::stoul(argv[1]);
    sf::SocketSelector selector;
    //char lastMessage[1000];
    std::string lastMessage;

    sf::TcpListener listener;
    if (listener.listen(port) != sf::Socket::Done) 
    {
        std::cerr << "Cannot listen on port " << port << std::endl;
        return 1;
    }
    selector.add(listener);

    std::thread connectionThread(buildConnection, std::ref(listener), std::ref(clients), 
        std::ref(selector), std::ref(lastMessage));
    connectionThread.detach();

    std::thread commandThread(userCommands, std::ref(clients), std::ref(lastMessage));

    if (commandThread.joinable()) {
        commandThread.join(); // 确保在退出前加入线程
    }
    
    if (connectionThread.joinable()) {
        connectionThread.join(); // 确保在退出前加入线程
    }

    for (auto* client : clients) {
        selector.remove(*client);
        client->disconnect();
        delete client;
    }

    return 0;
}
