using System;
using System.IO;
using static VhdRwCore.Constants;

namespace VhdRwCore
{
	abstract class VirtualHardDisk : IDisposable
	{
		public abstract byte[] ReadSector(long address);
		public abstract void WriteSector(long address, ReadOnlySpan<byte> data);

		protected FileInfo vhdFile;
		protected VhdFooter footer;
		protected FileStream vhdstream;
		private bool disposedValue;

		protected VirtualHardDisk(FileInfo vhdfile, VhdFooter footer)
		{
			vhdFile = vhdfile;
			this.footer = footer;
			vhdstream = vhdfile.Open(FileMode.Open, FileAccess.Read, FileShare.None);
		}

		public static VirtualHardDisk MountDisk(FileInfo vhdfile)
		{
			var buffer = new byte[SectorSize];
			using (var vhdstream = vhdfile.OpenRead())
			{
				//In case of legacy 511 byte footer. Footer should be sector aligned regardless of its size
				var sectoroffset = vhdstream.Position % SectorSize;
				if (sectoroffset == 0) sectoroffset = SectorSize;

				vhdstream.Seek(-sectoroffset, SeekOrigin.End);
				vhdstream.Read(buffer);
			}

			VhdFooter footer = new VhdFooter(buffer);

			if (footer.DiskType == DiskType.Fixed) return new FixedVhd(vhdfile, footer);
			else if (footer.DiskType == DiskType.Dynamic) return new DynamicVhd(vhdfile, footer);
			else if (footer.DiskType == DiskType.Differencing) throw new NotImplementedException("Differencing disks not yet supported");
			else throw new InvalidDataException("Unrecognized disk type");
		}

		public void Unmount()
		{
			vhdstream.Close();
		}

		protected virtual void Dispose(bool disposing)
		{
			if (!disposedValue)
			{
				if (disposing)
				{
					vhdstream.Dispose();
				}

				disposedValue = true;
			}
		}

		public void Dispose()
		{
			Dispose(true);
			GC.SuppressFinalize(this);
		}
	}
}
