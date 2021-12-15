#USAGE = python3.6 splitter_freq.py data_store.dat
import sys

name = str(sys.argv[1])	    

bench_freq_names = []
path0 = "./%s"%name 	#path0 = ./data_store.dat
print("the input file is ",path0) 
read_data_log = open(path0,'r')
header=read_data_log.readline()
new_line = read_data_log.readline()

data_values = new_line.split()

bench_name = data_values[1]

freq_value = data_values[3]

path_out = path0.split("/")

path_out = path_out[-1].split(".")

print("the benchmark name is ",bench_name)
print("the freq value is ",freq_value) 

bench_freq_names.append([bench_name,freq_value])
output_file = path_out[0]+"_"+bench_name+"_"+freq_value+".txt"

modify_output_file = open(output_file,'a')
print(header.rstrip('\n'),file=modify_output_file)
print(new_line.rstrip('\n'),file=modify_output_file)

for new_line in read_data_log: #read lines one by one
	data_values = new_line.split()

	if (data_values[1] == bench_name and data_values[3] == freq_value):
		print(new_line.rstrip('\n'),file=modify_output_file)

	else: #new_bench_name or new freq_value
		modify_output_file.close() #close file
		bench_name = data_values[1] #update bench
		freq_value = data_values[3] #update freq
		print("the benchmark name is ",bench_name)
		print("the freq value is ",freq_value)

		output_file = path_out[0]+"_"+bench_name+"_"+freq_value+".txt"
		modify_output_file = open(output_file,'a')

		if [bench_name,freq_value] not in bench_freq_names:
			bench_freq_names.append([bench_name,freq_value])
			print(header.rstrip('\n'),file=modify_output_file)
		print(new_line.rstrip('\n'),file=modify_output_file)
read_data_log.close()
modify_output_file.close()


