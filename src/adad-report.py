#!/usr/bin/python3
#
# Copyright (C) Université Paris-Saclay, UVSQ
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# adad-report: read the RAPL samples from a binary file and computes the mean,
# standard deviation, and percentage error of the energy consumption.

if __name__ == "__main__":
    data = []
    with open("rapl_samples.bin", "rb") as f:
        # first line is the RAPL domain
        domain = f.readline().decode('utf-8').strip()
        # second line is the scaling factor
        scaling = float(f.readline())
        # then samples are written as 8 bytes little endian integers 
        while True:
            bytes = f.read(8)
            if not bytes:
                break
            data.append(int.from_bytes(bytes, byteorder='little', signed=False)*scaling)

    # compute the mean and standard deviation
    mean = sum(data) / len(data)
    stddev = (sum((x - mean) ** 2 for x in data) / len(data)) ** 0.5
    percentage_error = (stddev / mean) * 100

    # print the mean with +- stddev / percentage_error and number of samples
    print(f"{domain} mean: {mean:.4f} ± {stddev:.4f} ({percentage_error:.2f}%) Joules from {len(data)} samples")