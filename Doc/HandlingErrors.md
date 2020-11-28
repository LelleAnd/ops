# Handling Errors #
It is possible to be notified if an error occurs in any of the OPS core threads.
the `ops::Participant` class accepts `ops::Listener<ops::Error*>` interfaces which will be notified with an `ops::Error` when an error occurs. The following shows how to implement such a listener and add it to the participant. Example given in C++:

```
class MyErrorListener : public Listener<ops::Error*>
{
public:
	void onNewEvent(ops::Notifier<ops::Error*>* notifier, ops::Error* error)
	{
		// do something with error
		error->getErrorCode();
		error->getMessage();
	}

};

...

MyErrorListener listener;
participant->addListener(&listener);

```

For the purpose of only logging errors, OPS' C++ implementation comes with a predefined class called `ops::ErrorWriter` which can be used to print to any `std::ostream`. This is how you use it:

```
ops::ErrorWriter errorWriter(std::cout);
participant->addListener(&errorWriter);
```

The above code will have all errors reported printed to the console with a timestamp.
