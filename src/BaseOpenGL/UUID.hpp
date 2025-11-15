#pragma once

template <typename T> class UUID
{
public:
  UUID() : _uuid(_ct++) {}

  inline int get() const { return _uuid; }

private:
    int _uuid = -1;
    static int _ct;
};

template <typename T> int UUID<T>::_ct = 0;