#ifndef PROTOCOL_H
#define PROTOCOL_H

#pragma once

typedef void (*Tcallback)(char *resource, const char *data);

class Protocol
{
public:
  virtual bool connect(const char *identifier, int port = 0) = 0;
  virtual void get(const char *resource, Tcallback callback) = 0;
  virtual void put(const char *resource, const char *payload) = 0;
  virtual bool isConnected() = 0;
};
#endif