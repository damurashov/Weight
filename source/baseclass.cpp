class baseclass {
protected:
  int mydata;
public:
  baseclass( void *arg ) {
    mydata = 0;
  }
  virtual int getMydata( ) = 0;
  virtual void putMydata( int data ) = 0;
};

typedef baseclass *instantiate_t( void *arg );
typedef void destroy_t( baseclass* );
