#!/usr/bin/python3

import csv
import sys, getopt

def read_file(filename):
	content_array = []
	with open(filename) as f:
		for line in f:
			content_array.append(line)
	return(content_array)

def get_samples_count(content_array):
	count = 0
	for  line in content_array:
		if "for" not in line:
			continue
		count = count + 1
	return count

def get_desired_response(sliced_array):
	for line in sliced_array:
		l = line.split(": ")
		if l[0] != "Total":
			continue
		return l[1].split(" ")[3]

def get_total_time(sliced_array):
	for line in sliced_array:
		l = line.split(": ")
		if l[0] != "Total":
			continue
		return l[1].split(" ")[7]

def get_errors_response(sliced_array):
	for line in sliced_array:
		l = line.split(": ")
		if l[0] != "Errors":
			continue
		return l[1].split(" ")[1]

def get_response_time(sliced_array):
	for line in sliced_array:
		l = line.split(": ")
		if l[0] != "Reply time [ms]":
			continue
		return l[1].split(" ")[1]	

def get_desired_response_pairs(content_array):
	num_samples = get_samples_count(content_array)
	for i in range(num_samples):
		sliced_array = content_array[ i*24 : (i+1)*24]
		desired = ((i*100)+100)*10 #get_desired_response(sliced_array)
		real = desired - int(get_errors_response(sliced_array))
		print(f"Real: {real}, desired: {desired}")

def get_desired_errors(content_array):
	num_samples = get_samples_count(content_array)
	for i in range(num_samples):
		sliced_array = content_array[ i*24 : (i+1)*24]
		desired = ((i*100)+100)*10 #get_desired_response(sliced_array)
		errors = int(get_errors_response(sliced_array))
		print(f"Errors: {errors}, desired: {desired}")
def get_data(content_array):
	num_samples = get_samples_count(content_array)
	row = "desired, real, errors, response_time, total_time"
	data_list = []
	data_list.append(row)
	for i in range(num_samples):
		sliced_array = content_array[ i*24 : (i+1)*24]
		desired = ((i*100)+100)*10 #get_desired_response(sliced_array)
		errors = int(get_errors_response(sliced_array))
		real = desired - int(get_errors_response(sliced_array))
		response_time = get_response_time(sliced_array)
		total_time = get_total_time(sliced_array)
		row = (f"{desired}, {real}, {errors}, {response_time}, {total_time} ")
		data_list.append(row)
	return data_list


# get_desired_response_pairs( read_file("torn.txt"))
# get_desired_errors( read_file("torn.txt"))
# file_list=["cpp.txt","nodejs.txt"]
def get_csv(file_list):
	for f in file_list:
		print(f)
		print("***"*8)
		with open(f.split('.')[0]+'.csv', 'w', newline='') as csvfile:
			writer = csv.writer(csvfile, delimiter=',', quoting=csv.QUOTE_MINIMAL)
			data = get_data(read_file(f))			
			for row in data:
				s_row =row.split(', ')
				for i, s in enumerate(s_row):    
					s_row[i] = s.replace('.',',')
				writer.writerow(s_row)
		print("---"*8)
#total response count
#desired vs real response
#timed out response
#response time logarithmic response y scale
if __name__ == "__main__":
   get_csv(sys.argv[1:])