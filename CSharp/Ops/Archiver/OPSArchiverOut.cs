///////////////////////////////////////////////////////////
//  OPSArchiverOut.cs
//  Implementation of the Class OPSArchiverOut
//  Created on:      12-nov-2011 09:25:32
//  Author:
///////////////////////////////////////////////////////////

using System.Collections;
using System.Collections.Generic;

namespace Ops
{
	public class OPSArchiverOut : IArchiverInOut 
    {
		internal WriteByteBuffer writeBuf;
        private readonly bool optNonVirt;

		public OPSArchiverOut(WriteByteBuffer buf, bool optNonVirt)
        {
            this.writeBuf = buf;
            this.optNonVirt = optNonVirt;
        }

        public override bool IsOut()
        {
            return true;
        }

		/// 
		/// <param name="name"></param>
		/// <param name="v"></param>
		public override int Inout(string name, int v)
        {
            writeBuf.Write(v);
            return v;
        }

		/// 
		/// <param name="name"></param>
		/// <param name="v"></param>
		public override long Inout(string name, long v)
        {
            writeBuf.Write(v);
            return v;
        }

		/// 
		/// <param name="name"></param>
		/// <param name="v"></param>
		public override byte Inout(string name, byte v)
        {
            writeBuf.Write(v);
            return v;
        }

		/// 
		/// <param name="name"></param>
		/// <param name="v"></param>
		public override short Inout(string name, short v)
        {
            writeBuf.Write(v);
            return v;
        }

		/// 
		/// <param name="name"></param>
		/// <param name="v"></param>
		public override float Inout(string name, float v)
        {
            writeBuf.Write(v);
            return v;
        }

		/// 
		/// <param name="name"></param>
		/// <param name="v"></param>
		public override bool Inout(string name, bool v)
        {
            writeBuf.Write(v);
            return v;
        }

		/// 
		/// <param name="name"></param>
		/// <param name="v"></param>
		public override string Inout(string name, string v)
        {
            writeBuf.Write(v);
            return v;
        }

		/// 
		/// <param name="name"></param>
		/// <param name="v"></param>
		public override double Inout(string name, double v)
        {
            writeBuf.Write(v);
            return v;
        }

        /// 
        /// <param name="name"></param>
        /// <param name="v"></param>
        public override ISerializable Inout<T>(string name, ISerializable v)
        {
            string types = "";
            if (((OPSObject)v).IdlVersionMask != 0)
            {
                types = "0 ";
            }
            if (optNonVirt)
            {
                writeBuf.Write(types);
            }
            else
            {
                writeBuf.Write(types + ((OPSObject)v).GetTypesString());
            }
            v.Serialize(this);
            return v;
        }

        /// 
        /// <param name="name"></param>
        /// <param name="v"></param>
        public override ISerializable Inout(string name, ISerializable v)
        {
            string types = "";
            if (((OPSObject)v).IdlVersionMask != 0)
            {
                types = "0 ";
            }
            writeBuf.Write(types + ((OPSObject)v).GetTypesString());
            v.Serialize(this);
            return v;
        }

		/// 
		/// <param name="name"></param>
		/// <param name="v"></param>
		public override List<bool> InoutBooleanList(string name, List<bool> v)
        {
            writeBuf.WriteBooleanArr(v);
            return v;
        }

		/// 
		/// <param name="name"></param>
		/// <param name="v"></param>
		public override List<byte> InoutByteList(string name, List<byte> v)
        {
            writeBuf.WriteByteArr(v);
            return v;
        }

		/// 
		/// <param name="name"></param>
		/// <param name="v"></param>
		public override List<double> InoutDoubleList(string name, List<double> v)
        {
            writeBuf.WriteDoubleArr(v);
            return v;
        }

		/// 
		/// <param name="name"></param>
		/// <param name="v"></param>
		public override List<float> InoutFloatList(string name, List<float> v)
        {
            writeBuf.WriteFloatArr(v);
            return v;
        }

		/// 
		/// <param name="name"></param>
		/// <param name="v"></param>
		public override List<int> InoutIntegerList(string name, List<int> v)
        {
            writeBuf.WriteIntArr(v);
            return v;
        }

		/// 
		/// <param name="name"></param>
		/// <param name="v"></param>
		public override List<long> InoutLongList(string name, List<long> v)
        {
            writeBuf.WriteLongArr(v);
            return v;
        }

        // NB! we assume that the object is a List<X> where X implements ISerializable.
        public override IList InoutSerializableList(string name, IList v)
        {
            writeBuf.Write(((IList)v).Count);
            foreach (ISerializable obj in (IList)v)
            {
                string types = "";
                if (((OPSObject)obj).IdlVersionMask != 0)
                {
                    types = "0 ";
                }
                writeBuf.Write(types + ((OPSObject)obj).GetTypesString());
                obj.Serialize(this);
            }
            return v;
        }

        // NB! we assume that the object is a List<T> where T implements ISerializable.
        public override IList InoutSerializableList<T>(string name, IList v)
        {
            writeBuf.Write(((IList)v).Count);
            foreach (ISerializable obj in (IList)v)
            {
                string types = "";
                if (((OPSObject)obj).IdlVersionMask != 0)
                {
                    types = "0 ";
                }
                if (optNonVirt)
                {
                    writeBuf.Write(types);
                }
                else
                {
                    writeBuf.Write(types + ((OPSObject)obj).GetTypesString());
                }
                obj.Serialize(this);
            }
            return v;
        }

        /// 
		/// <param name="name"></param>
		/// <param name="v"></param>
		public override List<short> InoutShortList(string name, List<short> v)
        {
            writeBuf.WriteShortArr(v);
            return v;
        }

		/// 
		/// <param name="name"></param>
		/// <param name="v"></param>
		public override List<string> InoutStringList(string name, List<string> v)
        {
            writeBuf.WriteStringArr(v);
            return v;
        }

	}

}