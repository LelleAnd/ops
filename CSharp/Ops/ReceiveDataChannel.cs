///////////////////////////////////////////////////////////
//  ReceiveDataChannel.cs
//  Implementation of the Class ReceiveDataChannel
//  Created on:
//  Author:
///////////////////////////////////////////////////////////

using System;
using System.Runtime.CompilerServices;  // Needed for the "MethodImpl" synchronization attribute
using System.Threading;

namespace Ops
{
    public class ReceiveDataChannel : IObserver
    {
		private readonly byte [] bytes;
		private int bytesReceived = 0;
		private int expectedFragment = 0;
		private static int FRAGMENT_HEADER_SIZE = 14;
		private int fragmentSize;
		private bool started = false;
		private byte [] headerBytes;
		private Participant participant;
		private IReceiver receiver;
		private static int THREAD_COUNTER = 1;
		private readonly Topic topic;
        private ReceiveDataHandler owner;
        // Key used to identify rdc when kept in a container
        public string Key;

        public ReceiveDataChannel(Topic t, Participant part, IReceiver receiver, ReceiveDataHandler owner)
        {
            topic = t;
            bytes = new byte[t.GetSampleMaxSize()];
            headerBytes = new byte[FRAGMENT_HEADER_SIZE];
            participant = part;
            fragmentSize = Globals.MAX_SEGMENT_SIZE;
            this.receiver = receiver;
            this.owner = owner;
        }

        [MethodImpl(MethodImplOptions.Synchronized)]
        public void Start()
        {
            if (!started)
            {
                // Reset some variables so we can start fresh 
                bytesReceived = 0;
                expectedFragment = 0;
                //
                receiver.GetNewBytesEvent().AddObserver(this);
                receiver.Open();
                started = true;
                SetupTransportThread();
            }
        }

        [MethodImpl(MethodImplOptions.Synchronized)]
        public void Stop()
        {
            if (started)
            {
                receiver.GetNewBytesEvent().DeleteObserver(this);
                started = false;
                receiver.Close();
            }
        }

        [MethodImpl(MethodImplOptions.Synchronized)]
        private void OnNewBytes(int size)
        {
            try
            {
                // Debug support
                if (Globals.TRACE_RECEIVE)
                {
                    Logger.ExceptionLogger.LogMessage("TRACE: ReceiveDataChannel.OnNewBytes() [" + topic.GetName() + "], got " + size + " bytes");
                }

                bytesReceived += size - headerBytes.Length;

                ReadByteBuffer readBuf = new ReadByteBuffer(headerBytes);

                if (readBuf.CheckProtocol())
                {
                    int nrOfFragments = readBuf.ReadInt();
                    int currentFragment = readBuf.ReadInt();

                    if (currentFragment == (nrOfFragments - 1) && currentFragment == expectedFragment)
                    {
                        // We have received all bytes for a full message, let's deserialize and forward it
                        DeserializeAndForward(new ReadByteBuffer(bytes));
                        expectedFragment = 0;
                        bytesReceived = 0;
                    }
                    else
                    {
                        if (currentFragment == expectedFragment)
                        {
                            expectedFragment++;
                        }
                        else
                        {
                            // Not so good. Sample will be lost here.
                            if (Globals.REPORT_DATA_FRAGMENT_LOST_ERRORS)
                            {
                                Logger.ExceptionLogger.LogMessage(this.GetType().Name + ", Fragment error, sample lost");
                            }
                            expectedFragment = 0;
                            bytesReceived = 0;
                        }
                    }
                }
            }
            catch (System.IO.IOException)
            {
                expectedFragment = 0;
                bytesReceived = 0;
            }
        }

        private void DeserializeAndForward(ReadByteBuffer readBuf)
        {
            OPSArchiverIn archiverIn = new OPSArchiverIn(readBuf, this.participant.getObjectFactory());
            OPSMessage message = null;

            // If the proper typesupport has not been added, the message may have content, but some fields may be null. How do we handle this
            // How do we make the user realize that he needs to add typesupport?
            message = (OPSMessage) archiverIn.Inout("message", message);

            //Console.WriteLine("Bytes received = " + bytesReceived);
            //Console.WriteLine("Bytes read     = " + readBuf.position());

            if (message == null)
            {
                Logger.ExceptionLogger.LogMessage(this.GetType().Name + ", message was unexpectedly null");
                return;
            }
   
            if (message.GetData() == null)
            {
                Logger.ExceptionLogger.LogMessage(this.GetType().Name + ", message.getData() was unexpectedly null");
                return;
            }

            CalculateAndSetSpareBytes(message, readBuf, FRAGMENT_HEADER_SIZE, bytesReceived);

            string IP = "";
            int port = 0;
            receiver.GetSource(ref IP, ref port);
            message.SetSource(IP, port);

            try
            {
                owner.OnNewMessage(message);
            } catch (Exception ex)
            {
                Logger.ExceptionLogger.LogMessage(this.GetType().Name + ", Exception thrown in event notification thread " + ex.ToString());
            }
        }

        private void CalculateAndSetSpareBytes(OPSMessage message, ReadByteBuffer readBuf, int segmentPaddingSize, int bytesReceived)
        {
            //NOT needed since 'bytesReceived' already is compensated for all headers and 'readBuf' is all data except header data
            //// We must calculate how many unserialized segment headers we have and substract 
            //// that total header size from the size of spareBytes.
            //int nrOfSerializedBytes = readBuf.Position();
            //int totalNrOfSegments = (int) (bytesReceived /fragmentSize);
            //int nrOfSerializedSegements = (int) (nrOfSerializedBytes /fragmentSize);
            //int nrOfUnserializedSegments = totalNrOfSegments - nrOfSerializedSegements;

            int nrOfSpareBytes = bytesReceived - readBuf.Position(); ///See comment above: -(nrOfUnserializedSegments * segmentPaddingSize);

            if (nrOfSpareBytes > 0)
            {
                message.GetData().spareBytes = new byte[nrOfSpareBytes];
                // This will read the rest of the bytes as raw bytes and put 
                // them into spareBytes field of data.
                readBuf.ReadBytes(message.GetData().spareBytes);  
            }
        }

        public void Run()
        {
            while (started)
            {
                // Here we will wait until new data arrives or the thread is terminated.
                // This thread is a background thread and will automatically be terminated
                // when all foreground threads has been terminated.
                receiver.Receive(headerBytes, bytes, expectedFragment * (fragmentSize - FRAGMENT_HEADER_SIZE));
            }
            //Console.WriteLine("Leaving transport thread...");
        }

        private void SetupTransportThread()
        {
            Thread thread = new Thread(new ThreadStart(Run));
            thread.IsBackground = true;
            thread.Name = "TransportThread_" + topic.GetTransport() + "_" + THREAD_COUNTER;
            THREAD_COUNTER++;
            thread.Start();
        }

        public void Update(IObservable o, object arg)
        {
            OnNewBytes((int)arg);
        }

    }

}