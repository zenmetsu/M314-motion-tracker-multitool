#ifndef __Framebuffer_h_
#define _Framebuffer_h_


class Framebuffer {
    public:
        Framebuffer();

        void erase();

        void put(int x, int y, char color);

        char get(int x, int y);

        const char* getPixels();

    private:
        static const int WIDTH = 160;
        static const int HEIGHT = 128;

        char pixels_[WIDTH * HEIGHT];
};

class Scene {
    public:
        Scene()
        : current_(&buffers_[0]),
          next_(&buffers_[1])
    {}

    void put(int x, int y, char color) {
        next_->put(x, y, color);
    }

    char get(int x, int y) {
        return current_->get(x, y);
    }

    void erase(char which) {
        switch(which) {
            case 1 :
                next_->erase();
                break;
            case 2 :
                current_->erase();
                break;
            case 3 :
                next_->erase();
                current_->erase();
                break;
        }
    }

    void flip() {
      swap();
    }

    bool compare(int x, int y) {
      return (current_->get(x, y) == next_->get(x, y));
    }

    Framebuffer& getBuffer() { return *current_; }

private:
  void swap() {
    // Just switch the pointers.
    Framebuffer* temp = current_;
    current_ = next_;
    next_ = temp;
  }

  Framebuffer  buffers_[2];
  Framebuffer* current_;
  Framebuffer* next_;
};


#endif /*\ #ifndef __Framebuffer_h_ \*/
