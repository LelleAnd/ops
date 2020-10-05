///////////////////////////////////////////////////////////
//  BasicError.cs
//  Implementation of the Class BasicError
//  Created on:      12-nov-2011 09:25:29
//  Author:
///////////////////////////////////////////////////////////

using System;

namespace Ops 
{
	internal class BasicError : IError 
    {
        private static int BASIC_ERROR_CODE = 0;
        private readonly string className;
		private readonly string errorMessage;
		private readonly string methodName;

        public BasicError(string className, string methodName, string errorMessage)
        {
            this.className = className;
            this.methodName = methodName;
            this.errorMessage = errorMessage;
        }

        public int GetErrorCode()
        {
            return BASIC_ERROR_CODE;
        }

        public string GetErrorMessage()
        {
            return errorMessage;
        }

        public string GetSource()
        {
            return className + "." + methodName + "()";
        }
	}

    [Serializable()]
    public class CommException : System.Exception
    {
        public CommException() : base() { }
        public CommException(string message) : base(message) { }
        public CommException(string message, System.Exception inner) : base(message, inner) { }

        // A constructor is needed for serialization when an
        // exception propagates from a remoting server to the client. 
        protected CommException(System.Runtime.Serialization.SerializationInfo info,
            System.Runtime.Serialization.StreamingContext context) { }
    }

    [Serializable()]
    public class ConfigException : System.Exception
    {
        public ConfigException() : base() { }
        public ConfigException(string message) : base(message) { }
        public ConfigException(string message, System.Exception inner) : base(message, inner) { }

        // A constructor is needed for serialization when an
        // exception propagates from a remoting server to the client. 
        protected ConfigException(System.Runtime.Serialization.SerializationInfo info,
            System.Runtime.Serialization.StreamingContext context) { }
    }

    [Serializable()]
    public class IdlVersionException : System.Exception
    {
        public IdlVersionException() : base() { }
        public IdlVersionException(string message) : base(message) { }
        public IdlVersionException(string message, System.Exception inner) : base(message, inner) { }

        // A constructor is needed for serialization when an
        // exception propagates from a remoting server to the client. 
        protected IdlVersionException(System.Runtime.Serialization.SerializationInfo info,
            System.Runtime.Serialization.StreamingContext context) { }
    }

    [Serializable()]
    public class InvalidHeaderException : System.Exception
    {
        public InvalidHeaderException() : base() { }
        public InvalidHeaderException(string message) : base(message) { }
        public InvalidHeaderException(string message, System.Exception inner) : base(message, inner) { }

        // A constructor is needed for serialization when an
        // exception propagates from a remoting server to the client. 
        protected InvalidHeaderException(System.Runtime.Serialization.SerializationInfo info,
            System.Runtime.Serialization.StreamingContext context) { }
    }

    [Serializable()]
    public class OPSInvalidTopicException : System.Exception
    {
        public OPSInvalidTopicException() : base() { }
        public OPSInvalidTopicException(string message) : base(message) { }
        public OPSInvalidTopicException(string message, System.Exception inner) : base(message, inner) { }

        // A constructor is needed for serialization when an
        // exception propagates from a remoting server to the client. 
        protected OPSInvalidTopicException(System.Runtime.Serialization.SerializationInfo info,
            System.Runtime.Serialization.StreamingContext context) { }
    }

    [Serializable()]
    public class OPSReceiveException : System.Exception
    {
        public OPSReceiveException() : base() { }
        public OPSReceiveException(string message) : base(message) { }
        public OPSReceiveException(string message, System.Exception inner) : base(message, inner) { }

        // A constructor is needed for serialization when an
        // exception propagates from a remoting server to the client. 
        protected OPSReceiveException(System.Runtime.Serialization.SerializationInfo info,
            System.Runtime.Serialization.StreamingContext context) { }
    }

    [Serializable()]
    public class OPSTopicTypeMissmatchException : System.Exception
    {
        public OPSTopicTypeMissmatchException() : base() { }
        public OPSTopicTypeMissmatchException(string message) : base(message) { }
        public OPSTopicTypeMissmatchException(string message, System.Exception inner) : base(message, inner) { }

        // A constructor is needed for serialization when an
        // exception propagates from a remoting server to the client. 
        protected OPSTopicTypeMissmatchException(System.Runtime.Serialization.SerializationInfo info,
            System.Runtime.Serialization.StreamingContext context) { }
    }

    [Serializable()]
    public class QoSNotSupportedException : System.Exception
    {
        public QoSNotSupportedException() : base() { }
        public QoSNotSupportedException(string message) : base(message) { }
        public QoSNotSupportedException(string message, System.Exception inner) : base(message, inner) { }

        // A constructor is needed for serialization when an
        // exception propagates from a remoting server to the client. 
        protected QoSNotSupportedException(System.Runtime.Serialization.SerializationInfo info,
            System.Runtime.Serialization.StreamingContext context) { }
    }

    [Serializable()]
    public class ReceiveTimedOutException : System.Exception
    {
        public ReceiveTimedOutException() : base() { }
        public ReceiveTimedOutException(string message) : base(message) { }
        public ReceiveTimedOutException(string message, System.Exception inner) : base(message, inner) { }

        // A constructor is needed for serialization when an
        // exception propagates from a remoting server to the client. 
        protected ReceiveTimedOutException(System.Runtime.Serialization.SerializationInfo info,
            System.Runtime.Serialization.StreamingContext context) { }
    }

    [Serializable()]
    public class RequestReplyException : System.Exception
    {
        public RequestReplyException() : base() { }
        public RequestReplyException(string message) : base(message) { }
        public RequestReplyException(string message, System.Exception inner) : base(message, inner) { }

        // A constructor is needed for serialization when an
        // exception propagates from a remoting server to the client. 
        protected RequestReplyException(System.Runtime.Serialization.SerializationInfo info,
            System.Runtime.Serialization.StreamingContext context) { }
    }

    [Serializable()]
    public class SubscriberHandlerNotFound : System.Exception
    {
        public SubscriberHandlerNotFound() : base() { }
        public SubscriberHandlerNotFound(string message) : base(message) { }
        public SubscriberHandlerNotFound(string message, System.Exception inner) : base(message, inner) { }

        // A constructor is needed for serialization when an
        // exception propagates from a remoting server to the client. 
        protected SubscriberHandlerNotFound(System.Runtime.Serialization.SerializationInfo info,
            System.Runtime.Serialization.StreamingContext context) { }
    }
}