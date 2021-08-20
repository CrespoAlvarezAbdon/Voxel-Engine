#ifndef _INDEXBUFFER_
#define _INDEXBUFFER_

class Index_buffer
{

public:

	Index_buffer();
	void prepare_static(const unsigned int* data, unsigned int indices_count);

	void bind() const;
	void unbind() const;

	unsigned int indices_count() const noexcept;

	~Index_buffer();

private:

	unsigned int renderer_ID_;
	unsigned int indices_count_;

};

inline unsigned int Index_buffer::indices_count() const noexcept
{

	return indices_count_; 

}

#endif