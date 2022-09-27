using System;
using System.IO;
using System.Text;
using System.Linq;

namespace VhdRwCore
{
	class DynamicVhdHeader : DataBlock
	{
		private const string signature = "cxsparse";
		private const int cookieOffset = 0;
		private const int dataOffsetOffset = 8;
		private const int tableOffsetOffset = 16;
		private const int headerVersionOffset = 24;
		private const int maxTableEntriesOffset = 28;
		private const int blockSizeOffset = 32;
		private const int checksumOffset = 36;
		private const int parentIdOffset = 40;
		private const int parentTimeStampOffset = 56;
		private const int parentNameOffset = 64;
		private const int parentEntriesOffset = 576;
		private const int parentLocationEntrySize = 24;

		public string Cookie => ExtractString(cookieOffset, Encoding.ASCII, 8);
		private long DataOffset => ExtractNumber<long>(dataOffsetOffset); //unused per documentation
		public long TableOffset => ExtractNumber<long>(tableOffsetOffset);
		public Version HeaderVersion => ExtractVersion(headerVersionOffset);
		public int MaxTableEntries => ExtractNumber<int>(maxTableEntriesOffset);
		public int BlockSize => ExtractNumber<int>(blockSizeOffset);
		public int Checksum => ExtractNumber<int>(checksumOffset);
		public Guid ParentId => ExtractGuid(parentIdOffset);
		public DateTime ParentTimeStamp => ExtractTimeStamp(parentTimeStampOffset);
		public string ParentName => ExtractString(parentNameOffset, Encoding.Unicode, 512);
		public LocatorEntryCollection LocatorEntries => new LocatorEntryCollection(data.AsSpan(parentEntriesOffset, parentLocationEntrySize * 7));
		private bool ChecksumIsValid => ~Checksum == (data.Take(checksumOffset).Sum(c => c) + data.Skip(checksumOffset + 4).Sum(c => c));

		public DynamicVhdHeader(byte[] headerData) : base(headerData)
		{
			if (Cookie != signature || !ChecksumIsValid) throw new InvalidDataException("Signature or checksum invalid");
		}
	}
}
