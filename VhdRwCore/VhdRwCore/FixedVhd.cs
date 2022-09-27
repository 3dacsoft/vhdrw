using System;
using System.IO;
using static VhdRwCore.Constants;

namespace VhdRwCore
{
	internal class FixedVhd : VirtualHardDisk
	{
		public FixedVhd(FileInfo vhdfile, VhdFooter footer) : base(vhdfile, footer) { }

		public override byte[] ReadSector(long address)
		{
			var sector = new byte[SectorSize];
			vhdstream.Seek(address * SectorSize, SeekOrigin.Begin);
			vhdstream.Read(sector);
			return sector;
		}

		public override void WriteSector(long address, ReadOnlySpan<byte> data)
		{
			if (data.Length > SectorSize) throw new InvalidDataException();
			vhdstream.Seek(address * SectorSize, SeekOrigin.Begin);
			if (data.Length == SectorSize)
				vhdstream.Write(data);
			else
			{
				var buffer = new byte[SectorSize];
				data.CopyTo(buffer);
				vhdstream.Write(buffer);
			}
		}
	}
}
