using System;
using System.IO;
using static VhdRwCore.Constants;

namespace VhdRwCore
{
	class Program
	{
		static void Main(string[] args)
		{

			try
			{
				ValidateArgs(args, out DiskOperations operation, out int sectorNumber, out FileInfo vhdfile);

				var binfile = new FileInfo(args[3]);
				if (operation == DiskOperations.Write && !binfile.Exists) throw new ArgumentException("File not found", "binpath");

				using var vhd = VirtualHardDisk.MountDisk(vhdfile);

				if (operation == DiskOperations.Read)
				{
					using var binstream = binfile.OpenWrite();
					binstream.Write(vhd.ReadSector(sectorNumber));
				}
				else if(operation == DiskOperations.Write)
				{
					byte[] writedata = new byte[512];
					using (var bintream = binfile.OpenRead()) bintream.Read(writedata);
					vhd.WriteSector(sectorNumber, writedata);
				}
			}
			//Reads in the header/footer, evaluates disk type and returns correct subclass
			catch (ArgumentException ae)
			{
				Console.WriteLine("Bad argument: " + ae.Message + Environment.NewLine +
								  Environment.NewLine +
								  "Correct usage: vhdrw <vhdpath> <r/w> <sector number> <binpath>");
			}
			catch(Exception e)
			{
				Console.WriteLine("Something went wrong: " + e.ToString());
			}
		}

		private static void ValidateArgs(string[] args, out DiskOperations operation, out int sectorNumber, out FileInfo vhdfile)
		{
			var argcount = args.Length;
			if (argcount < MinArguments || argcount > MaxArguments)
				throw new ArgumentException("Wrong number of arguments: " + argcount);

			vhdfile = new FileInfo(args[0]);
			if (!vhdfile.Exists) throw new ArgumentException("File not found", "vhdpath");

			operation = (DiskOperations)args[1][0];
			if (!Enum.IsDefined(typeof(DiskOperations), operation))
				throw new ArgumentException("Invalid operation", "operation(r/w)");

			if (!int.TryParse(args[2], out sectorNumber))
				throw new ArgumentException("Unable to parse number", "sector number");
		}
	}
}
