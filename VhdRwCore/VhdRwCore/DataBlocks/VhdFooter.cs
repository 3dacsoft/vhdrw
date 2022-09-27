using System;
using System.IO;
using System.Text;
using System.Linq;

namespace VhdRwCore
{
	internal class VhdFooter : DataBlock
	{
		private const int CookieOffset = 0;
		private const int FeaturesOffset = 8;
		private const int FormatVersionOffset = 12;
		private const int DataOffsetOffset = 16;
		private const int TimeStampOffset = 24;
		private const int CreatorApplicationOffsetOffset = 28;
		private const int CreatorVersionOffset = 32;
		private const int CreatorHostOsOffset = 36;
		private const int OriginalSizeOffset = 40;
		private const int CurrentSizeOffset = 48;
		private const int DiskGeometryOffset = 56;
		private const int DiskTypeOffset = 60;
		private const int ChecksumOffset = 64;
		private const int UniqueIdOffset = 68;
		private const int SavedStateOffset = 84;

		public string Cookie => ExtractString(CookieOffset, Encoding.ASCII, 8);
		public VhdFeatures Features => (VhdFeatures)ExtractNumber<int>(FeaturesOffset);
		public Version FileFormatVersion => ExtractVersion(FormatVersionOffset);
		public long DataOffset => ExtractNumber<long>(DataOffsetOffset);
		public DateTime CreationTime => ExtractTimeStamp(TimeStampOffset);
		public string CreatorApplication => ExtractString(CreatorApplicationOffsetOffset, Encoding.ASCII, 4);
		public Version CreatorVersion => ExtractVersion(CreatorVersionOffset);
		public string CreateHostOs => ExtractString(CreatorHostOsOffset, Encoding.ASCII, 4);
		public long OriginalSize => ExtractNumber<long>(OriginalSizeOffset);
		public long CurrentSize => ExtractNumber<long>(CurrentSizeOffset);
		public DiskGeometry Geometry => new DiskGeometry(data.AsSpan(DiskGeometryOffset, 4));
		public DiskType DiskType => (DiskType)ExtractNumber<int>(DiskTypeOffset);
		public int Checksum => ExtractNumber<int>(ChecksumOffset);
		public Guid UniqueId => ExtractGuid(UniqueIdOffset);
		public bool SavedState => ExtractNumber<byte>(SavedStateOffset) == 1;
		private bool ChecksumIsValid => data.Take(ChecksumOffset).Sum(c => c) + data.Skip(UniqueIdOffset).Sum(c => c) == ~Checksum;

		public VhdFooter(byte[] footerBytes) : base(footerBytes)
		{
			if (!ChecksumIsValid)
				throw new InvalidDataException("Invalid checksum");
		}

	}

	[Flags]
	enum VhdFeatures
	{
		None = 2,
		Temporary = 3,
	}

	internal class DiskGeometry : DataBlock
	{
		public int Cylinders => ExtractNumber<short>(0);
		public int Heads => data[2];
		public int Sectors => data[3];

		public DiskGeometry(ReadOnlySpan<byte> bytes) : base(bytes.ToArray()) { }
	}

	enum DiskType
	{
		None = 0,
		Fixed = 2,
		Dynamic,
		Differencing,
	}
}
