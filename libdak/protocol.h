
namespace dak
{
	enum class message_type : unsigned char
	{
		// UP: Send a message to a topic.
		// DOWN: Send a message for a subscribed topic.
		// seqId(short) topic(string) message(string)
		ET_MESSAGE = 0,

		// UP: Subscribe a topic.
		// seqId(short) topic(string)
		ET_SUBSCRIBE = 2,

		// DOWN: ACK for any request.
		// seqId(short) errNo(byte)
		ET_ACK = 200,
	};
}
