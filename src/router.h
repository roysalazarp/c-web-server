#ifndef ROUTER_H
#define ROUTER_H

void route_request(int channel, const char* method, const char* url);
void home(int channel);
void not_found(int channel);

#endif