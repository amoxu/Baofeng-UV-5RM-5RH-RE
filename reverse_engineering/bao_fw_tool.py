#!/usr/bin/env python3
# Created by Pavel, OK2MOP, CC BY-SA
import struct
import sys
import os
import binascii

class bao_fw_tool(object):
	_package_size = 1024

	def __init__(self, key1=b"KDHT", key2=b"RBGI"):
		self.key1 = key1
		self.key2 = key2

	def xor_crypt(self, data, key):
		pos = 0
		odata = b""
		for x in data:
			if x != 0 and x != 0xff and \
			   x != key[pos % 4] and x != (key[pos % 4] ^ 0xff):
				x ^= key[pos % 4]
			pos += 1
			odata += struct.pack('B', x)
		return odata

	def crypt(self, data):
		packages = len(data) // self._package_size
		if (len(data) % self._package_size) > 0:
			packages += 1
		dec_buffer = data[:2*self._package_size]
		for i in range(2,packages-1):
			block = data[i*self._package_size:(i+1)* self._package_size]
			if i % 3 == 1 and i < packages - 2:
				dec_buffer += self.xor_crypt(block, self.key1)
			elif i % 3 == 2 and i < packages - 2:
				dec_buffer += self.xor_crypt(block, self.key2)
			else:
				dec_buffer += block
		index1 = 0
		dec_buffer += data[self._package_size * (packages-1):]
		dec_buffer = dec_buffer[:len(data)]
		return dec_buffer

	def split(self, input_f, output_p, decrypt=True):
		add = ""
		if decrypt:
			add = "-dec"
		try:
			f = open(input_f, "rb")
			hdr1 = f.read(16)
			parts = struct.unpack('>BLL7B', hdr1)
			size1 = parts[1]
			size2 = parts[2]

			#First part
			oname = output_p + add +"-1.bin"
			of = open(oname, "wb")
			data1 = f.read(size1)
			if decrypt:
				data1 = self.crypt(data1)
			of.write(data1)
			of.close()

			#Second part
			oname = output_p + add + "-2.bin"
			of = open(oname, "wb")
			f.seek(16+size1,0)
			data2 = f.read(size2)
			if decrypt:
				data2 = self.crypt(data2)
			of.write(data2)
			of.close()

		except FileNotFoundError:
			sys.stderr.write("Input file %s cannot be opened\n" % input_f)
		except OSError:
			sys.stderr.write("Output file %s cannot be opened\n" % oname)

	def merge(self, input_p, output_f, encrypt=True):
		if encrypt:
			add = "-dec"
		try:
			#First part
			iname = input_p + add + "-1.bin"
			f = open(iname, "rb")
			data1 = f.read()
			f.close()
			if encrypt:
				data1 = self.crypt(data1)

			#Second part
			iname = input_p + add + "-2.bin"
			f = open(iname, "rb")
			data2 = f.read()
			f.close()
			if encrypt:
				data2 = self.crypt(data2)

			# Create output file
			of = open(output_f, "wb")
			hdr = struct.pack('>BLL', 0x02, len(data1), len(data2))
			#Bytes 9-16 in header seem to be ignored by flasher
			hdr += 7 * b'\xff'
			of.write(hdr)
			of.write(data1)
			of.write(data2)
			of.close()

		except FileNotFoundError:
			sys.stderr.write("Input file %s cannot be opened\n" % input_f)
		except OSError:
			sys.stderr.write("Output file %s cannot be opened\n" % oname)

if __name__ == "__main__":
	args = sys.argv
	show_help = False
	pack = False
	crypt = True
	prog = os.path.basename(args[0])
	if len(args) == 4:
		mode=args[1]
		if mode[:4].upper() == "PACK":
			pack = True
		elif mode[:6].upper() == "UNPACK":
			pack = False
		else:
			show_help = True
		if mode[-1:] == "E":
			crypt = True
		inp  = args[2]
		outp = args[3]
	else:
		show_help = True

	tool = bao_fw_tool()
	if show_help:
		sys.stderr.write(prog + " mode input output\n")
		sys.stderr.write("Where: mode can be PACK (packs decrypted files), PACK_E\n")
		sys.stderr.write("                   UNPACK, UNPACK_E (keeps encryption) \n")
		sys.stderr.write("       input is a .BF firmware file for unpacking or prefix with -{1,2}.bin for packing\n")
		sys.stderr.write("       output is a .BF firmware file for packing or prefix with {1,2}.bin for unpacking\n")
		sys.stderr.write("\ne.g. `" + prog + " UNPACK example.BF out' will create out-dec-1.bin and out-dec-2.bin \n")
	elif pack:
		tool.merge(inp, outp, crypt)
	else:
		tool.split(inp, outp, crypt)
