# Threading and Event Callbacks #

The threading model of OPS differs a bit between C++ and other language implementations and will therefore be described one by one below:

## Threading Model C++ ##

The C++ implementation of OPS uses *boost.asio* as the core for its communication and thread model. *boost.asio* allows for asynchronous socket io and enables each ops.Participant to run on a single thread. This thread asynchronously handle receptions of datagrams, handles deadline timeouts and timers that periodically fire up.

More than doing work in the core of OPS, this thread also occasionally finds its way into your application code. This happens when your are notified of new data or deadlineTimeouts via listeners. It is important to understand that if you hold up the executing thread in such a listener callback, this will affect the whole participant. Listener callbacks should always do as little work as possible, if you need to do more heavy work, consider polling the subscriber instead or transfer your work to another worker thread.


## Threading Model C#, Java, Delphi, Ada and Python ##

In C#, Java, Delphi, Ada and Python, each unique port in the configuration yields one thread that waits on the socket for new data to arrive. Deadline timeouts are checked by another thread common for the whole participant.

Both these threads also occasionally finds its ways into your application code. This happens when your are notified of new data or deadlineTimeouts via listeners. It is important to understand that if you hold up the executing thread in such a listener callback, this will affect all subscribers to topics using that port within the participant (on new data events) or the whole participant (deadline events). Listener callbacks should always do as little work as possible, if you need to do more heavy work, consider polling the subscriber instead or transfer your work to another worker thread.

To understand more about the threads of OPS, please have a look at the developer documentation and the source code it self.
