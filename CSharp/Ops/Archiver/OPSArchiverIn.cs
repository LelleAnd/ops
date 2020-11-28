///////////////////////////////////////////////////////////
//  OPSArchiverIn.cs
//  Implementation of the Class OPSArchiverIn
//  Created on:      12-nov-2011 09:25:32
//  Author:
///////////////////////////////////////////////////////////

using System;
using System.Collections;
using System.Collections.Generic;

namespace Ops 
{
	public class OPSArchiverIn : IArchiverInOut 
    {
		private SerializableCompositeFactory compositeFactory;
		private ReadByteBuffer readBuf;

		/// 
		/// <param name="buf"></param>
		public OPSArchiverIn(ReadByteBuffer buf)
        {
            compositeFactory = OPSObjectFactory.GetInstance();
            readBuf = buf;
		}

        /// <summary>
        /// 
        /// </summary>
        /// <param name="buf"></param>
        /// <param name="factory"></param>
        public OPSArchiverIn(ReadByteBuffer buf, OPSObjectFactory factory)
        {
            compositeFactory = factory;
            readBuf = buf;
        }

        public override bool IsOut()
        {
            return false;
        }

		/// 
		/// <param name="e"></param>
		public void Add(ISerializableFactory item)
        {
            compositeFactory.Add(item);
        }

		/// 
		/// <param name="name"></param>
		/// <param name="v"></param>
		public override int Inout(string name, int v)
        {
            return readBuf.ReadInt();
        }

		/// 
		/// <param name="name"></param>
		/// <param name="v"></param>
		public override long Inout(string name, long v)
        {
            return readBuf.ReadLong();
        }

		/// 
		/// <param name="name"></param>
		/// <param name="v"></param>
		public override byte Inout(string name, byte v)
        {
            return readBuf.ReadByte();
        }

		/// 
		/// <param name="name"></param>
		/// <param name="v"></param>
		public override short Inout(string name, short v)
        {
            return readBuf.ReadShort();
		}

		/// 
		/// <param name="name"></param>
		/// <param name="v"></param>
		public override float Inout(string name, float v)
        {
            return readBuf.ReadFloat();
        }

		/// 
		/// <param name="name"></param>
		/// <param name="v"></param>
		public override bool Inout(string name, bool v)
        {
            return readBuf.ReadBoolean();
        }

		/// 
		/// <param name="name"></param>
		/// <param name="v"></param>
		public override string Inout(string name, string v)
        {
            return readBuf.ReadString();
        }

		/// 
		/// <param name="name"></param>
		/// <param name="v"></param>
		public override double Inout(string name, double v)
        {
            return readBuf.ReadDouble();
        }

        /// 
        /// <param name="name"></param>
        /// <param name="v"></param>
        public override ISerializable Inout<T>(string name, ISerializable v)
        {
            string types = readBuf.ReadString();  // Skip string for non virtual objects, type is known by T
            ISerializable newSer = (ISerializable) new T();
            if ((types.Length > 0) && (types[0] == '0'))
            {
                ((OPSObject)newSer).IdlVersionMask = 1;     // Some value != 0
            }
            newSer.Serialize(this);
            return newSer;
        }

        /// 
        /// <param name="name"></param>
        /// <param name="v"></param>
        public override ISerializable Inout(string name, ISerializable v)
        {
            int verMask = 0;
            string types = readBuf.ReadString();
            if ((types.Length > 0) && (types[0] == '0'))
            {
                verMask = 1;
                types = types.Substring(2);
            }
            ISerializable newSer = compositeFactory.Create(types);
            if (newSer != null)
            {
                //Do this to preserve type information even if slicing has occured.
                (newSer as OPSObject).SetTypesString(types);

                (newSer as OPSObject).IdlVersionMask = verMask;
                newSer.Serialize(this);
            }
            return newSer;
        }

		/// 
		/// <param name="name"></param>
		/// <param name="v"></param>
		public override List<bool> InoutBooleanList(string name, List<bool> v)
        {
            return readBuf.ReadBooleanArr();
        }

		/// 
		/// <param name="name"></param>
		/// <param name="v"></param>
		public override List<byte> InoutByteList(string name, List<byte> v)
        {
            return readBuf.ReadByteArr();
        }

		/// 
		/// <param name="name"></param>
		/// <param name="v"></param>
		public override List<double> InoutDoubleList(string name, List<double> v)
        {
            return readBuf.ReadDoubleArr();
        }

		/// 
		/// <param name="name"></param>
		/// <param name="v"></param>
		public override List<float> InoutFloatList(string name, List<float> v)
        {
            return readBuf.ReadFloatArr();
        }

		/// 
		/// <param name="name"></param>
		/// <param name="v"></param>
		public override List<int> InoutIntegerList(string name, List<int> v)
        {
            return readBuf.ReadIntArr();
        }

		/// 
		/// <param name="name"></param>
		/// <param name="v"></param>
		public override List<long> InoutLongList(string name, List<long> v)
        {
            return readBuf.ReadLongArr();
        }

        // NB! we assume that the object is a List<X> where X implements ISerializable.
        public override IList InoutSerializableList(string name, IList v)
        {
            Type type = v.GetType().GetGenericArguments()[0];
            IList list = (IList)Activator.CreateInstance((typeof(List<>).MakeGenericType(type)));
            int size = readBuf.ReadInt();
            for (int i = 0; i < size; i++)
            {
                list.Add(Inout("", (ISerializable)null));
            }
            return list;
        }

        // NB! we assume that the object is a List<T> where T implements ISerializable.
        // This method skips the factory for creating the elements 
        public override IList InoutSerializableList<T>(string name, IList v)
        {
            Type type = v.GetType().GetGenericArguments()[0];
            IList list = (IList)Activator.CreateInstance((typeof(List<>).MakeGenericType(type)));
            int size = readBuf.ReadInt();
            for (int i = 0; i < size; i++)
            {
                // For each element
                string types = readBuf.ReadString();
                int verMask = 0;
                if ((types.Length > 0) && (types[0] == '0'))
                {
                    verMask = 1;
                    types = types.Substring(2);
                }

                ISerializable obj = (ISerializable)Activator.CreateInstance<T>();
                (obj as OPSObject).IdlVersionMask = verMask;
                obj.Serialize(this);
                list.Add(obj);
            }
            return list;
        }

        /// 
		/// <param name="name"></param>
		/// <param name="v"></param>
		public override List<short> InoutShortList(string name, List<short> v)
        {
            return readBuf.ReadShortArr();
        }

		/// 
		/// <param name="name"></param>
		/// <param name="v"></param>
		public override List<string> InoutStringList(string name, List<string> v)
        {
            return readBuf.ReadStringArr();
        }

		/// 
		/// <param name="o"></param>
        public void remove(ISerializableFactory item)
        {
            compositeFactory.Remove(item);
        }

	}
}