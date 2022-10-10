import time
import numpy as np
import tensorflow as tf
import pandas as pd
from pandas import DataFrame
import concurrent.futures
import csv
import networkx as nx
import multiprocessing
import itertools
import time
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import json
import glob
import os
import natsort
from natsort import natsorted
import copy
import pickle
import random
import zipfile
from statistics import mean
from collections import namedtuple

from tensorflow.keras import layers
from tensorflow.keras import activations

from sklearn.datasets import make_multilabel_classification
from sklearn.model_selection import train_test_split
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Dense
from tensorflow.keras.losses import binary_crossentropy
from tensorflow.keras.optimizers import Adam
import tensorflow.keras.backend as K
from functools import partial, update_wrapper
from sklearn.metrics import accuracy_score
from sklearn.metrics import confusion_matrix
from sklearn.metrics import classification_report
from tensorflow import keras
pd.set_option('mode.chained_assignment', None)

#Create input table
def create_input(x1,y1,z1):
    dfjobs=[]
    dfMess=[]
    dfNodes=[]
    dfLinks=[]
    dfApplication=[]
    dfPlatform=[]
    dfFinalTable=[]
    jobs=[]
    messages=[]
    nodes = DataFrame(NewInputData[0]['platform']['nodes'])
    links = DataFrame(NewInputData[0]['platform']['links'])
    for i in range(x1,y1):# extracting the elements of each json file and appending together
        dfjobs.append(NewInputData[i]['application']['jobs'])
        dfMess.append(NewInputData[i]['application']['messages'])
    for i in range(0,(y1-x1)): #creating application table
        #dfApplication.append(pd.concat([DataFrame(dfjobs[i]),DataFrame(dfMess[i])],axis = 1)) 
        jobs.append(DataFrame(dfjobs[i]))
        messages.append(DataFrame(dfMess[i]))
    #for i in range(0,(y1-x1)): #puting them together
     #   dfFinalTable.append(DataFrame(dfApplication[i]))
    for j in range(z1):#dropping unwanted columns from input
        jobs[j]=jobs[j].drop(['deadline','mcet'],axis= 'columns')
        messages[j]=messages[j].drop(['timetriggered', 'period'], axis= 'columns')
        #dfFinalTable[j]=dfFinalTable[j].drop(['deadline','mcet','timetriggered','period'],axis= 'columns')
        #dfFinalTable[j]=dfFinalTable[j].iloc[:,~dfFinalTable[j].columns.duplicated()]
    return jobs, messages, nodes, links

#Create output table
def create_output(x1,y1,z1):
    outjobs=[]
    outMess=[]
    outOrder=[]
    outApplication=[]
    makespan = []
    for i in range(x1,y1):# extracting the elements of each json file and appending together
        outjobs.append(NewOutputData[i]['jobs']) 
        outMess.append(NewOutputData[i]['messages'])
        makespan.append(NewOutputData[i]['makespan'])
    for i in range(0,(y1-x1)): #creating output table
        #outApplication.append(pd.concat([DataFrame(outjobs[i]),DataFrame(outMess[i])],axis = 1))
        outApplication.append(DataFrame(outjobs[i]))
    for j in range(z1):#dropping unwanted columns from output
        #outApplication[j]=outApplication[j].drop(['id','can_run_on','deadline','frequency','wcet','wcet_with_compression','compressed_size','receiver','route','route_idx','scheme_idx','sender','size','timetriggered','start_time','injection_time$
        outApplication[j]=outApplication[j].drop(['id','can_run_on','deadline','frequency','wcet','wcet_with_compression','start_time','runs_on'],axis= 'columns')
    return outApplication, makespan

#NEW function to add children and parents
def add_child_parent(jobs, messages, x1):
    dfjobs = copy.deepcopy(jobs)
    table_with_child_parent=[]
    for k in range(x1):
      child_all = []
      parent_all = []
      for i in jobs[k].id:
        child = []
        parent = []
        for j in messages[k].id:
          if i == messages[k].sender[j]:
            child.append(messages[k].receiver[j])
          if i == messages[k].receiver[j]:
            parent.append(messages[k].sender[j])
        child_all.append(child)
        parent_all.append(parent)
      dfjobs[k].insert(2, "parents", parent_all, True) 
      dfjobs[k].insert(3, "children", child_all, True) 
      table_with_child_parent.append(dfjobs[k])
    return table_with_child_parent

#New function to calculate and add t_level
def add_t_level(jobs, messages, x1):
    top_level_value = []
    for i in range(len(jobs[0])):
        top_level_value.append(0)
    dfjobs= copy.deepcopy(jobs)
    for k in range(x1):
      dfjobs[k].insert(4, "t_level", top_level_value, True)
      for i in reversed(range(0,len(jobs[0]))):
        dfjobs[k].t_level[i] = t_level(dfjobs[k], messages[k], i)
    return dfjobs

#New function to calculate and add b_level
def add_b_level(jobs, messages, x1):
    bottom_level_value = []
    for i in range(len(jobs[0])):
        bottom_level_value.append(0)
    dfjobs= copy.deepcopy(jobs)
    for k in range(x1):
      dfjobs[k].insert(5, "b_level", bottom_level_value, True)
      for i in range(len(jobs[0])):
        dfjobs[k].b_level[i] = b_level(dfjobs[k], messages[k], i)
    return dfjobs

#New function to get the size of the message
def message_size(table,nx,ni):
  index_list = table[(table['sender'] == nx)&(table['receiver'] == ni)].index.tolist()
  size = int(table.iloc[index_list]['size'])
  return size

#Functions to calculate b_level 
def b_level(jobs, messages, job):
  max = 0
  for ny in jobs.children[job]:
    if jobs.b_level[ny] == 0:
      if message_size(messages, job, ny) + b_level(jobs, messages, ny) > max:
        max = message_size(messages, job, ny) + b_level(jobs, messages, ny)
    else:
      if message_size(messages, job, ny) + jobs.b_level[ny] > max:
        max = message_size(messages, job, ny) + jobs.b_level[ny]
  return jobs.wcet_fullspeed[job] + max

#New Function to calculate t_level 
def t_level(jobs, messages, job):
  max = 0
  for ny in jobs.parents[job]:
    if jobs.t_level[ny] == 0:
      if message_size(messages, ny, job) + t_level(jobs, messages, ny) + jobs.wcet_fullspeed[ny] > max:
        max = message_size(messages, ny, job) + t_level(jobs, messages, ny) + jobs.wcet_fullspeed[ny]
    else:
      if message_size(messages, ny, job) + jobs.t_level[ny] + jobs.wcet_fullspeed[ny] > max:
        max = message_size(messages, ny, job) + jobs.t_level[ny] + jobs.wcet_fullspeed[ny]
  return max

#Max message size
def max_msg_size(_input_table, job, x, k):
    if x == 1:
        if job in _input_table[k].sender.values:
                size = max(_input_table[k].loc[_input_table[k]['sender'] == job]['size'])
        else:
                size = 0
    else:
        if job in _input_table[k].receiver.values:
                size = max(_input_table[k].loc[_input_table[k]['receiver'] == job]['size'])
        else:
                size = 0
    return size

#Order
def ordering(input_table1, x1, feature):
    finalList1 = []
    for i in range(x1):
        if feature == 0:                                #Order by t_level
            input_table1[i] = input_table1[i].sort_values(by ='t_level' , ascending=True)
        else:                                           #Order by id
            input_table1[i] = input_table1[i].sort_values(by ='id' , ascending=True)
        finalList1.append(input_table1[i])
    return finalList1

#Merge input and output tables
def add_OuttoIn(input_table, output_table, x1):
        InOut = []
        for i in range(x1): 
                InOut.append(pd.concat([DataFrame(input_table[i]), DataFrame(output_table[i])], axis = 1))
        return InOut

#Reindexing
def Re_index(_input_table1, x1):
    finalList1 = []
    for i in range(x1):
        _input_table1[i].reset_index(drop=True, inplace=True)
        finalList1.append(_input_table1[i])
    return finalList1

#Merge input and output tables
def add_OuttoIn(input_table, output_table, x1):
        InOut = []
        for i in range(x1): 
                InOut.append(pd.concat([DataFrame(input_table[i]), DataFrame(output_table[i])], axis = 1))
        return InOut

#Extracting features
def oneD_features(jobs, messages, samples):
    Input = []
    for j in range(samples):
        Job = []
        et_total = jobs[j]['wcet_fullspeed'].sum()
        highest_msg_size = messages[j]['size'].max()
        for i in range (len(jobs[0])):
            f = []
            f.append(jobs[j].index[i] / len(jobs[0]))                                        #id / #Jobs
            f.append(jobs[j].wcet_fullspeed[i] / et_total)                            #Ex.time / Total execution
            f.append(max_msg_size(messages,i, 1, j) / highest_msg_size)                                           #Maximum size message sent / Highest size
            f.append(max_msg_size(messages,i, 0, j) / highest_msg_size)                                           #Maximum size message receiver / Highest size
            f.append(len(messages[j][messages[j].sender == i]) / len(messages[0]))  #Messages sent / #Messages
            f.append(len(messages[j][messages[j].receiver == i]) / len(messages[0]))  #Messages received / #Messages
            f.append(jobs[j].t_level[i] / jobs[j]['t_level'].max())                              #t_level / Max (t_level)
            f.append(jobs[j].b_level[i] / jobs[j]['b_level'].max())                              #b_level / Max (b_level)
            Job.append(f)
        Input.append(Job)        
    return Input

#Creating the labels
def label_encoding(_size):
        num_jobs = 10
        labels = []
        for s in range(_size):
                label_vector = []
                for i in range(num_jobs-1):
                        for j in range(i+1, num_jobs):
                                if finalTable[s].position[i] > finalTable[s].position[j]:
                                #if finalTable[s].weight[i] < finalTable[s].weight[j]:   #weight
                                        label_vector.append(1)
                                else:
                                         label_vector.append(0)
                labels.append(label_vector)
        return labels

#reading the input and output files from directory
print("Reading files from directory")
inputfiles = glob.glob('Inputs_ccr_0.6_10/*.json', recursive=True) #'NAME of Directory'
print("Total number of inputs: ", len(inputfiles))
outputfiles = glob.glob('Outputs_ccr_0.6_10/*.json', recursive=True)#'NAME of Directory'
print("Total number of outputs: ", len(outputfiles))
print("Done") 

#sorting the files to have a sequence
print("Sorting files")
inputfiles = natsorted(inputfiles)
outputfiles = natsorted(outputfiles)
print("Done")

#combinig the Input json files into one json file
print("Combining input json files")
inputdata = []
for single_file in inputfiles:
    with open(single_file, 'r') as f:
        inputdata.append(json.load(f))

#combinig the Output json files into one json file
print("Combining output json files")
outputdata = []
for single_outputfile in outputfiles:
        with open(single_outputfile, 'r') as o:
                outputdata.append(json.load(o))
print("Done")

#Saving the input merged json files as a new .json file
with open("merged_file.json", "w") as Infile:
    json.dump(inputdata, Infile)
#Opening the saved .json file and storing it in a new variable for later usages in the program
with open('merged_file.json') as InNEW:
    NewInputData = json.load(InNEW)

#Saving the output merged json files as a new .json file
with open("merged_outputfile.json", "w") as outputfile:
    json.dump(outputdata, outputfile)

#Opening the saved .json file and storing it in a new variable for later usages in the program
with open('merged_outputfile.json') as outNEW:
        NewOutputData = json.load(outNEW)

no_tables = 20000

total = no_tables
y1 = int(total/10)
y2 = int(total/10)*2
y3 = int(total/10)*3
y4 = int(total/10)*4
y5 = int(total/10)*5
y6 = int(total/10)*6
y7 = int(total/10)*7
y8 = int(total/10)*8            
y9 = int(total/10)*9
y10 = total

n1 = y1-0
n2 = y2-y1 
n3 = y3-y2
n4 = y4-y3
n5 = y5-y4 
n6 = y6-y5
n7 = y7-y6
n8 = y8-y7
n9 = y9-y8
n10 = y10-y9
print("Range: ", 0, "to", y1)
print("Range: ", y1+1, "to", y2)
print("Range: ", y2+1, "to", y3)
print("Range: ", y3+1, "to", y4)
print("Range: ", y4+1, "to", y5)
print("Range: ", y5+1, "to", y6)
print("Range: ", y6+1, "to", y7)
print("Range: ", y7+1, "to", y8)
print("Range: ", y8+1, "to", y9)
print("Range: ", y9+1, "to", y10)

#Saving output data in a dataframes
print("Extracting information from the outputs")
start = time.time()
with concurrent.futures.ProcessPoolExecutor() as executer:
        outputf1 = executer.submit(create_output,0,y1,n1)
        outputf2 = executer.submit(create_output,y1,y2,n2)
        outputf3 = executer.submit(create_output,y2,y3,n3)
        outputf4 = executer.submit(create_output,y3,y4,n4)
        outputf5 = executer.submit(create_output,y4,y5,n5)
        outputf6 = executer.submit(create_output,y5,y6,n6)
        outputf7 = executer.submit(create_output,y6,y7,n7)
        outputf8 = executer.submit(create_output,y7,y8,n8)
        outputf9 = executer.submit(create_output,y8,y9,n9)
        outputf10 = executer.submit(create_output,y9,y10,n10)
outputtable1, makespan_GA1 = outputf1.result()
outputtable2, makespan_GA2 = outputf2.result()
outputtable3, makespan_GA3 = outputf3.result()
outputtable4, makespan_GA4 = outputf4.result()
outputtable5, makespan_GA5 = outputf5.result()
outputtable6, makespan_GA6 = outputf6.result()
outputtable7, makespan_GA7 = outputf7.result()
outputtable8, makespan_GA8 = outputf8.result()
outputtable9, makespan_GA9 = outputf9.result()
outputtable10, makespan_GA10 = outputf10.result()

end = time.time()
print(f'\n time: {end - start :.2f}s\n')
print("Done")

#Saving input data in a dataframes
print("Extracting information from the inputs")
start = time.time()
with concurrent.futures.ProcessPoolExecutor() as executer:
        inputf1 = executer.submit(create_input,0,y1,n1)
        inputf2 = executer.submit(create_input,y1,y2,n2)
        inputf3 = executer.submit(create_input,y2,y3,n3)
        inputf4 = executer.submit(create_input,y3,y4,n4)
        inputf5 = executer.submit(create_input,y4,y5,n5)
        inputf6 = executer.submit(create_input,y5,y6,n6)
        inputf7 = executer.submit(create_input,y6,y7,n7)
        inputf8 = executer.submit(create_input,y7,y8,n8)
        inputf9 = executer.submit(create_input,y8,y9,n9)
        inputf10 = executer.submit(create_input,y9,y10,n10)
jobs1, messages1, nodes, links = inputf1.result()
jobs2, messages2, nodes, links = inputf2.result()
jobs3, messages3, nodes, links = inputf3.result()
jobs4, messages4, nodes, links = inputf4.result()
jobs5, messages5, nodes, links = inputf5.result()
jobs6, messages6, nodes, links = inputf6.result()
jobs7, messages7, nodes, links = inputf7.result()
jobs8, messages8, nodes, links = inputf8.result()
jobs9, messages9, nodes, links = inputf9.result()
jobs10, messages10, nodes, links = inputf10.result()
end = time.time()
print(f'\n time: {end - start :.2f}s\n')
print("Done")

#Adding children and parent column inside the input dataframes
print("Adding children and parent column")
start = time.time()
with concurrent.futures.ProcessPoolExecutor() as executer:
        add_in1 = executer.submit(add_child_parent,jobs1,messages1,n1)
        add_in2 = executer.submit(add_child_parent,jobs2,messages2,n2)
        add_in3 = executer.submit(add_child_parent,jobs3,messages3,n3)
        add_in4 = executer.submit(add_child_parent,jobs4,messages4,n4)
        add_in5 = executer.submit(add_child_parent,jobs5,messages5,n5)
        add_in6 = executer.submit(add_child_parent,jobs6,messages6,n6)
        add_in7 = executer.submit(add_child_parent,jobs7,messages7,n7)
        add_in8 = executer.submit(add_child_parent,jobs8,messages8,n8)
        add_in9 = executer.submit(add_child_parent,jobs9,messages9,n9)
        add_in10 = executer.submit(add_child_parent,jobs10,messages10,n10)
children_parent1 = add_in1.result()
children_parent2 = add_in2.result()
children_parent3 = add_in3.result()
children_parent4 = add_in4.result()
children_parent5 = add_in5.result()
children_parent6 = add_in6.result()
children_parent7 = add_in7.result()
children_parent8 = add_in8.result()
children_parent9 = add_in9.result()
children_parent10 = add_in10.result()
end = time.time()
print(f'\n time: {end - start :.2f}s\n')
print("Done")

#Adding top level calculation column for each job inside the input dataframes
print("Adding t_level column")
start = time.time()
with concurrent.futures.ProcessPoolExecutor() as executer:
       table_with_t_value1 = executer.submit(add_t_level,children_parent1,messages1,n1)
       table_with_t_value2 = executer.submit(add_t_level,children_parent2,messages2,n2)
       table_with_t_value3 = executer.submit(add_t_level,children_parent3,messages3,n3)
       table_with_t_value4 = executer.submit(add_t_level,children_parent4,messages4,n4)
       table_with_t_value5 = executer.submit(add_t_level,children_parent5,messages5,n5)
       table_with_t_value6 = executer.submit(add_t_level,children_parent6,messages6,n6)
       table_with_t_value7 = executer.submit(add_t_level,children_parent7,messages7,n7)
       table_with_t_value8 = executer.submit(add_t_level,children_parent8,messages8,n8)
       table_with_t_value9 = executer.submit(add_t_level,children_parent9,messages9,n9)
       table_with_t_value10 = executer.submit(add_t_level,children_parent10,messages10,n10)
end = time.time()
print(f'\n time: {end - start :.2f}s\n')
t_level_table1 = table_with_t_value1.result()
t_level_table2 = table_with_t_value2.result()
t_level_table3 = table_with_t_value3.result()
t_level_table4 = table_with_t_value4.result()
t_level_table5 = table_with_t_value5.result()
t_level_table6 = table_with_t_value6.result()
t_level_table7 = table_with_t_value7.result()
t_level_table8 = table_with_t_value8.result()
t_level_table9 = table_with_t_value9.result()
t_level_table10 = table_with_t_value10.result()
print("Done")

#Adding bottom level calculation column for each job inside the input dataframes
print("Adding b_level column")
start = time.time()
with concurrent.futures.ProcessPoolExecutor() as executer:
        table_with_b_value1 = executer.submit(add_b_level,t_level_table1,messages1,n1)
        table_with_b_value2 = executer.submit(add_b_level,t_level_table2,messages2,n2)
        table_with_b_value3 = executer.submit(add_b_level,t_level_table3,messages3,n3)
        table_with_b_value4 = executer.submit(add_b_level,t_level_table4,messages4,n4)
        table_with_b_value5 = executer.submit(add_b_level,t_level_table5,messages5,n5)
        table_with_b_value6 = executer.submit(add_b_level,t_level_table6,messages6,n6)
        table_with_b_value7 = executer.submit(add_b_level,t_level_table7,messages7,n7)
        table_with_b_value8 = executer.submit(add_b_level,t_level_table8,messages8,n8)
        table_with_b_value9 = executer.submit(add_b_level,t_level_table9,messages9,n9)
        table_with_b_value10 = executer.submit(add_b_level,t_level_table10,messages10,n10)
b_level_table1 = table_with_b_value1.result()
b_level_table2 = table_with_b_value2.result()
b_level_table3 = table_with_b_value3.result()
b_level_table4 = table_with_b_value4.result()
b_level_table5 = table_with_b_value5.result()
b_level_table6 = table_with_b_value6.result()
b_level_table7 = table_with_b_value7.result()
b_level_table8 = table_with_b_value8.result()
b_level_table9 = table_with_b_value9.result()
b_level_table10 = table_with_b_value10.result()
end = time.time()
print(f'\n time: {end - start :.2f}s\n')
print("Done")

#Merging input and output dataframes
print("Merging input and output tables")
start = time.time()
with concurrent.futures.ProcessPoolExecutor() as executer:
        add_In_Out1 = executer.submit(add_OuttoIn,b_level_table1,outputtable1,n1)
        add_In_Out2 = executer.submit(add_OuttoIn,b_level_table2,outputtable2,n2)
        add_In_Out3 = executer.submit(add_OuttoIn,b_level_table3,outputtable3,n3)
        add_In_Out4 = executer.submit(add_OuttoIn,b_level_table4,outputtable4,n4)
        add_In_Out5 = executer.submit(add_OuttoIn,b_level_table5,outputtable5,n5)
        add_In_Out6 = executer.submit(add_OuttoIn,b_level_table6,outputtable6,n6)
        add_In_Out7 = executer.submit(add_OuttoIn,b_level_table7,outputtable7,n7)
        add_In_Out8 = executer.submit(add_OuttoIn,b_level_table8,outputtable8,n8)
        add_In_Out9 = executer.submit(add_OuttoIn,b_level_table9,outputtable9,n9)
        add_In_Out10 = executer.submit(add_OuttoIn,b_level_table10,outputtable10,n10)
InOutTable1 = add_In_Out1.result()
InOutTable2 = add_In_Out2.result()
InOutTable3 = add_In_Out3.result()
InOutTable4 = add_In_Out4.result()
InOutTable5 = add_In_Out5.result()
InOutTable6 = add_In_Out6.result()
InOutTable7 = add_In_Out7.result()
InOutTable8 = add_In_Out8.result()
InOutTable9 = add_In_Out9.result()
InOutTable10 = add_In_Out10.result()
end = time.time()
print(f'\n time: {end - start :.2f}s\n')
print("Done")

#Ordering the jobs in each dataframe based on their bottom level value
print("Ordering based on t_level")
start = time.time()
with concurrent.futures.ProcessPoolExecutor() as executer:
        ordering_t_level1 = executer.submit(ordering,InOutTable1,len(InOutTable1),0)
        ordering_t_level2 = executer.submit(ordering,InOutTable2,len(InOutTable2),0)
        ordering_t_level3 = executer.submit(ordering,InOutTable3,len(InOutTable3),0)
        ordering_t_level4 = executer.submit(ordering,InOutTable4,len(InOutTable4),0)
        ordering_t_level5 = executer.submit(ordering,InOutTable5,len(InOutTable5),0)
        ordering_t_level6 = executer.submit(ordering,InOutTable6,len(InOutTable6),0)
        ordering_t_level7 = executer.submit(ordering,InOutTable7,len(InOutTable7),0)
        ordering_t_level8 = executer.submit(ordering,InOutTable8,len(InOutTable8),0)
        ordering_t_level9 = executer.submit(ordering,InOutTable9,len(InOutTable9),0)
        ordering_t_level10 = executer.submit(ordering,InOutTable10,len(InOutTable10),0)
OrderTable1 = ordering_t_level1.result()
OrderTable2 = ordering_t_level2.result()
OrderTable3 = ordering_t_level3.result()
OrderTable4 = ordering_t_level4.result()
OrderTable5 = ordering_t_level5.result()
OrderTable6 = ordering_t_level6.result()
OrderTable7 = ordering_t_level7.result()
OrderTable8 = ordering_t_level8.result()
OrderTable9 = ordering_t_level9.result()
OrderTable10 = ordering_t_level10.result()
end = time.time()
print(f'\n time: {end - start :.2f}s\n')
print("Done")

#Reindexing
print("Resetting the index of the tables")
start = time.time()
with concurrent.futures.ProcessPoolExecutor() as executer:
        reindex1 = executer.submit(Re_index,OrderTable1,len(OrderTable1))
        reindex2 = executer.submit(Re_index,OrderTable2,len(OrderTable2))
        reindex3 = executer.submit(Re_index,OrderTable3,len(OrderTable3))
        reindex4 = executer.submit(Re_index,OrderTable4,len(OrderTable4))
        reindex5 = executer.submit(Re_index,OrderTable5,len(OrderTable5))
        reindex6 = executer.submit(Re_index,OrderTable6,len(OrderTable6))
        reindex7 = executer.submit(Re_index,OrderTable7,len(OrderTable7))
        reindex8 = executer.submit(Re_index,OrderTable8,len(OrderTable8))
        reindex9 = executer.submit(Re_index,OrderTable9,len(OrderTable9))
        reindex10 = executer.submit(Re_index,OrderTable10,len(OrderTable10))
new_index1 = reindex1.result()
new_index2 = reindex2.result()
new_index3 = reindex3.result()
new_index4 = reindex4.result()
new_index5 = reindex5.result()
new_index6 = reindex6.result()
new_index7 = reindex7.result()
new_index8 = reindex8.result()
new_index9 = reindex9.result()
new_index10 = reindex10.result()
end = time.time()
print(f'\n time: {end - start :.2f}s\n')
print("Done")

#Ordering based on the id
print("Ordering based on the id")
start = time.time()
with concurrent.futures.ProcessPoolExecutor() as executer:
    ordering_id1 = executer.submit(ordering,new_index1,len(new_index1),1)
    ordering_id2 = executer.submit(ordering,new_index2,len(new_index2),1)
    ordering_id3 = executer.submit(ordering,new_index3,len(new_index3),1)
    ordering_id4 = executer.submit(ordering,new_index4,len(new_index4),1)
    ordering_id5 = executer.submit(ordering,new_index5,len(new_index5),1)
    ordering_id6 = executer.submit(ordering,new_index6,len(new_index6),1)
    ordering_id7 = executer.submit(ordering,new_index7,len(new_index7),1)
    ordering_id8 = executer.submit(ordering,new_index8,len(new_index8),1)
    ordering_id9 = executer.submit(ordering,new_index9,len(new_index9),1)
    ordering_id10 = executer.submit(ordering,new_index10,len(new_index10),1)
end = time.time()
print(f'\n time: {end - start :.2f}s\n')
Order_id1 = ordering_id1.result()
Order_id2 = ordering_id2.result()
Order_id3 = ordering_id3.result()
Order_id4 = ordering_id4.result()
Order_id5 = ordering_id5.result()
Order_id6 = ordering_id6.result()
Order_id7 = ordering_id7.result()
Order_id8 = ordering_id8.result()
Order_id9 = ordering_id9.result()
Order_id10 = ordering_id10.result()
print("Done")


#Reindexing
print("Resetting the index of the tables")
start = time.time()
with concurrent.futures.ProcessPoolExecutor() as executer:
    re_index1 = executer.submit(Re_index,Order_id1,len(Order_id1))
    re_index2 = executer.submit(Re_index,Order_id2,len(Order_id2))
    re_index3 = executer.submit(Re_index,Order_id3,len(Order_id3))
    re_index4 = executer.submit(Re_index,Order_id4,len(Order_id4))
    re_index5 = executer.submit(Re_index,Order_id5,len(Order_id5))
    re_index6 = executer.submit(Re_index,Order_id6,len(Order_id6))
    re_index7 = executer.submit(Re_index,Order_id7,len(Order_id7))
    re_index8 = executer.submit(Re_index,Order_id8,len(Order_id8))
    re_index9 = executer.submit(Re_index,Order_id9,len(Order_id9))
    re_index10 = executer.submit(Re_index,Order_id10,len(Order_id10))
end = time.time()
print(f'\n time: {end - start :.2f}s\n')
index1 = re_index1.result()
index2 = re_index2.result()
index3 = re_index3.result()
index4 = re_index4.result()
index5 = re_index5.result()
index6 = re_index6.result()
index7 = re_index7.result()
index8 = re_index8.result()
index9 = re_index9.result()
index10 = re_index10.result()
print("Done")


#Obtaining 1D features
print("Obtaining 1D features")
start = time.time()
with concurrent.futures.ProcessPoolExecutor() as executer:
       features_1D_1 = executer.submit(oneD_features,index1,messages1,len(index1))
       features_1D_2 = executer.submit(oneD_features,index2,messages2,len(index2))
       features_1D_3 = executer.submit(oneD_features,index3,messages3,len(index3))
       features_1D_4 = executer.submit(oneD_features,index4,messages4,len(index4))
       features_1D_5 = executer.submit(oneD_features,index5,messages5,len(index5))
       features_1D_6 = executer.submit(oneD_features,index6,messages6,len(index6))
       features_1D_7 = executer.submit(oneD_features,index7,messages7,len(index7))
       features_1D_8 = executer.submit(oneD_features,index8,messages8,len(index8))
       features_1D_9 = executer.submit(oneD_features,index9,messages9,len(index9))
       features_1D_10 = executer.submit(oneD_features,index10,messages10,len(index10))
end = time.time()
print(f'\n time: {end - start :.2f}s\n')
OneD_feature1 = features_1D_1.result()
OneD_feature2 = features_1D_2.result()
OneD_feature3 = features_1D_3.result()
OneD_feature4 = features_1D_4.result()
OneD_feature5 = features_1D_5.result()
OneD_feature6 = features_1D_6.result()
OneD_feature7 = features_1D_7.result()
OneD_feature8 = features_1D_8.result()
OneD_feature9 = features_1D_9.result()
OneD_feature10 = features_1D_10.result()
print("Done")

#Joining the features from the different processes
OneD_Features = OneD_feature1 + OneD_feature2 + OneD_feature3 + OneD_feature4 + OneD_feature5 + OneD_feature6 + OneD_feature7 + OneD_feature8 + OneD_feature9 + OneD_feature10
print("Size of features",len(OneD_Features))

#Saving the features in a csv file
print("Saving features in csv file")
Input_ = np.array(OneD_Features)
Input_ = Input_.reshape(-1, 80)
Input_ = pd.DataFrame(Input_)
Input_.to_csv('features_10.csv', index = False, header = False)
print("Done")

#Joining the output dataframes (lists of dataframes) from the different processes
finalTable = index1 + index2 + index3 + index4 + index5 + index6 + index7 + index8 + index9 + index10
total_messages = messages1 + messages2 + messages3 + messages4 + messages5 + messages6 + messages7 + messages8 + messages9 + messages10 

print("Size of final list of tables",len(finalTable))
pickle.dump(finalTable, open("table_jobs.p", 'wb'))
pickle.dump(total_messages, open("table_messages.p", 'wb'))
pickle.dump(nodes, open("nodes.p", 'wb'))
pickle.dump(links, open("links.p", 'wb'))

#Extracting labels
print("Extracting and saving labels")
start = time.time()
labels = label_encoding(len(finalTable))
labels = np.array(labels).flatten()
labels = labels.reshape(-1,45)
with open('labels_10.csv', 'w', newline='') as file:
       myfile = csv.writer(file)
       myfile.writerows(labels)
end = time.time()
print(f'\n time: {end - start :.2f}s\n')
print("Done")


#---------------------------------------------------------------

#NN model

pd.set_option('mode.chained_assignment', None)
startT = time.time()

def weighted_binary_crossentropy():
    """A weighted binary crossentropy loss function
    that works for multilabel classification
    """
    # obtain dataset here
    data = train_labels#full_dataset_here[indices]
    # create a 2 by N array with weights for 0's and 1's
    weights = np.zeros((2, data.shape[1]))
    # calculates weights for each label in a for loop
    for i in range(data.shape[1]):
        weights_n, weights_p = (data.shape[0]/(2 * (data[:,i] == 0).sum()+100)), (data.shape[0]/(2 * (data[:,i] == 1).sum()+100))
        # weights could be log-dampened to avoid extreme weights for extremly unbalanced data.
        weights[1, i], weights[0, i] = weights_p, weights_n

    # The below is needed to be able to work with keras' model.compile()
    def wrapped_partial(func, *args, **kwargs):
        partial_func = partial(func, *args, **kwargs)
        update_wrapper(partial_func, func)
        return partial_func

    def wrapped_weighted_binary_crossentropy(y_true, y_pred, class_weights):
        y_pred = K.clip(y_pred, K.epsilon(), 1.0-K.epsilon())
        # cross-entropy loss with weighting
        out = -(y_true * K.log(y_pred)*class_weights[1] + (1.0 - y_true) * K.log(1.0 - y_pred)*class_weights[0])
        return K.mean(out, axis=-1)

    return wrapped_partial(wrapped_weighted_binary_crossentropy, class_weights=weights)

def loss_fn(y_true,y_pred):
    loss = 0
    for i in range(45):
        loss -= (positive_weights[i]*y_true[i]*K.log(y_pred[i]) + negative_weights[i]*(1-y_true[i])*K.log(1-y_pred[i]))
    return loss

#Importing features from csv file
features_csv = pd.read_csv('features_10.csv', header=None)
features = features_csv.to_numpy() #Converting it to numpy array
print(features.shape)

#Importing labels from csv file
labels_csv = pd.read_csv('labels_10.csv', header=None)
labels = labels_csv.to_numpy() #Converting it to numpy array
print(features.shape)

#Dividing features and labels into train and test data
train_features, test_features, train_labels, test_labels = train_test_split(features, labels, test_size = 0.2, shuffle = False )
inputs= tf.keras.Input(shape=80)
flat = tf.keras.layers.Flatten()(inputs)
first_layer = tf.keras.layers.Dense(units=100, input_dim = 80)(flat)
relu_layer = tf.keras.layers.ReLU()(first_layer)

output_layer = tf.keras.layers.Dense(units = 45, input_dim=100)(relu_layer)
sigmoid_layer = tf.keras.layers.Activation('sigmoid')(output_layer)

model = tf.keras.Model(inputs = inputs, outputs = sigmoid_layer, name="lua")
#model = Sequential()
#first_layer = Dense(100, input_dim=80)
#model.add(first_layer)
#model.add(layers.Activation(activations.relu))
#model.add(Dense(100, activation='relu', input_dim=80))
#output_layer = Dense(45)
#model.add(output_layer)
#model.add(layers.Activation(activations.sigmoid))
#model.add(Dense(45, activation='sigmoid'))
print(model)

custom_loss = weighted_binary_crossentropy()
#Compile the model
opt = keras.optimizers.Adam(learning_rate = 0.001)
model.compile(loss=custom_loss, optimizer=opt, metrics=['binary_accuracy'])
#model.compile(loss='binary_crossentropy', optimizer=opt, metrics=['binary_accuracy'])

#Fit data to model
model.fit(train_features, train_labels, batch_size=128, epochs=300, verbose=1, validation_split=0.1, shuffle = False )

score = model.evaluate(test_features, test_labels, verbose = 0)
model.trainable = False
print(f'Test loss: {score[0]} / Test accuracy: {score[1]}')
model.save("model_10.h5")
print("Prediction vectors")
predictions = model.predict(test_features, batch_size = 128, verbose = 0)
#Round probabilities to class labels
#predictions = predictions.round()
predictions = (predictions > 0.54).astype(int)
#Calculate accuracy
acc = accuracy_score(test_labels, predictions)
print("accuracy score",acc)

print(predictions.shape)
print(type(predictions))

print("Predictions in csv")
with open('predictions_priorities.csv', 'w', newline='') as file:
    myfile = csv.writer(file)
    myfile.writerows(predictions)
print("Done")

print("Test labels in csv")
with open('real_priorities.csv', 'w', newline='') as file:
    myfile = csv.writer(file)
    myfile.writerows(test_labels)
print("Done")

print(classification_report(test_labels, predictions))


#Getting the Data Ready Time
def DRT(processor, job, messages, jobs):
    DRT_parents = []
    for j in enumerate(jobs.parents[job]):
        if len(jobs.parents[job]) == 1:
            DRT_parents.append(0)
        DRT_parents.append(fT(j[1], jobs) + comm_cost(j[1], job, processor, messages, jobs))
    return max(DRT_parents)

#Finish time
def fT(job, _input_table):
    return (_input_table.end_time[job])
  
#Get the communication costs based on the physical model (This might need to change) 
def comm_cost(parent, job, processor, messages, jobs): 
    b = messages.index[(messages.sender == parent) & (messages.receiver == job)].tolist()
    if abs(processor - jobs.processor[parent]) == 1:
        cost = (2 * messages['size'][b[0]])
    elif abs(processor - jobs.processor[parent]) == 0:
        cost = 0
    elif abs(processor - jobs.processor[parent]) in range(2, 5):
        if processor == 1 | jobs.processor[parent] == 1 | processor == 2 | jobs.processor[parent] == 2:
            cost = (3 * messages['size'][b[0]])
        else: 
            cost = (4 * messages['size'][b[0]])
    elif abs(processor - jobs.processor[parent]) in range(5, 9):
        cost = (3 * messages['size'][b[0]])
    return cost

#Series sum
def sum_series(n):
    return n * (n+1) / 2

#Allocation algorithm
def allocation(vector, jobs, messages, _end_systems, x1):
    _input_table = copy.deepcopy(jobs)
    finalTable = []
    makespan = []
    for i in range(x1):
        locks = []
        jobs_ready = []
        jobs_done = []
        processor_endTime = [0 for j in range(len(_end_systems))]                                                       #The vector will save the current end times on each processor
        for j in range (len(_input_table[i])):
            if not _input_table[i].parents[j]:
                jobs_ready.append(j)
        for k in range(10):
            max_b_level_id = 0
            max_b_level_value = 0
            processor = 0

            for id, value in enumerate(jobs_ready):
                if _input_table[i].b_level[value] >= max_b_level_value:
                    max_b_level_id = _input_table[i].id[value]
                    max_b_level_value = _input_table[i].b_level[value]
            allocated_job = max_b_level_id

            copy_vector = copy.deepcopy(jobs_ready)
            reps = []
            for elemx in range(len(jobs_ready)-1):
              a = min(copy_vector)
              copy_vector.remove(a)
              index_vector = int(sum_series(9) - sum_series(9-a))
              if len(copy_vector) > 0:
                for elemy in copy_vector:
                  #print("index", index_vector+elemy-a-1)
                  #print("value_index",vector[i][index_vector+elemy-a-1])
                  if vector[i][index_vector+elemy-a-1] == 1:
                    reps.append(elemy)
                  else:
                    reps.append(a)
              else:
                print("Just one job:", a)
            print("rep",reps)
            if reps:
              allocated_job = max(reps, key = reps.count)
              #print("This is allocated", max(reps, key = reps.count))
            else:
              allocated_job = jobs_ready[0]#a#min(jobs_ready)
              #print("This is allocated(empty)", allocated_job)

            
            #print("jobs_ready", jobs_ready)
            #print("really alocated", allocated_job)
            

#Here comes the algorithm that deicdes in which processor will be allocated
            if k == 0:
                _input_table[i].processor[allocated_job] = _end_systems[0]
                _input_table[i].end_time[allocated_job] = _input_table[i].wcet_fullspeed[allocated_job]
                processor_endTime[0] = _input_table[i].end_time[allocated_job] 
            else:
                start_time = []
                for l in enumerate(_end_systems):
                    if not _input_table[i].parents[allocated_job]:
                        start_time.append(processor_endTime[l[0]])
                    else:
                        start_time.append(max(processor_endTime[l[0]], DRT(l[1], allocated_job, messages[i], _input_table[i])))
                _input_table[i].start_time[allocated_job] = min(start_time)
                _input_table[i].end_time[allocated_job] = _input_table[i].start_time[allocated_job] + _input_table[i].wcet_fullspeed[allocated_job]
                _input_table[i].processor[allocated_job] = _end_systems[start_time.index(min(start_time))]

                if _input_table[i].parents[allocated_job]:
                    arrival = []
                    for msg in _input_table[i].parents[allocated_job]:
                        injection = find_and_lock(_input_table[i].end_time[msg], int(messages[i]['size'][messages[i].index[(messages[i].sender == msg) & (messages[i].receiver == allocated_job)].tolist()]), locks, routing100(_input_table[i].processor[msg], _input_table[i].processor[allocated_job]), routing100(_input_table[i].processor[msg], _input_table[i].processor[allocated_job]) )
                        arrival.append(injection + comm_cost(msg, allocated_job, _input_table[i].processor[allocated_job], messages[i], _input_table[i]))
                    _input_table[i].start_time[allocated_job] = max(max(arrival),  processor_endTime[start_time.index(min(start_time))])
                    _input_table[i].end_time[allocated_job] = _input_table[i].start_time[allocated_job] + _input_table[i].wcet_fullspeed[allocated_job]
                    processor_endTime[start_time.index(min(start_time))] = _input_table[i].end_time[allocated_job]
                ##
                else:
                    processor_endTime[start_time.index(min(start_time))] = _input_table[i].end_time[allocated_job]
            print("k",k)
            jobs_ready.remove(allocated_job)
            jobs_done.append(allocated_job)
            children = _input_table[i].children[allocated_job]
            for l in enumerate(children):
                if set(_input_table[i].parents[l[1]]).issubset(jobs_done):
                    jobs_ready.append(l[1])
            jobs_ready = list(set(jobs_ready))
        finalTable.append(_input_table[i])
        makespan.append(max(_input_table[i].end_time))
        
    return finalTable, makespan

#Routing path
def routing100(sender, receiver):
    if (sender == 4 and receiver == 1) or (sender == 5 and receiver == 1):
        route = [3, 0, 1]
    elif (sender == 4 and receiver == 2) or (sender == 5 and receiver == 2):
        route = [3, 0, 2]
    elif (sender == 4 and receiver == 7) or (sender == 5 and receiver == 7):
        route = [3, 0, 6, 7]
    elif (sender == 4 and receiver == 8) or (sender == 5 and receiver == 8):
        route = [3, 0, 6, 8]
    elif (sender == 1 and receiver == 4) or (sender == 2 and receiver == 4):
        route = [0, 3, 4]
    elif (sender == 1 and receiver == 5) or (sender == 2 and receiver == 5):
        route = [0, 3, 5]
    elif (sender == 1 and receiver == 7) or (sender == 2 and receiver == 7):
        route = [0, 6, 7]
    elif (sender == 1 and receiver == 8) or (sender == 2 and receiver == 8):
        route = [0, 6, 8]
    elif (sender == 7 and receiver == 1) or (sender == 8 and receiver == 1):
        route = [6, 0, 1]
    elif (sender == 7 and receiver == 2) or (sender == 8 and receiver == 2):
        route = [6, 0, 2]
    elif (sender == 7 and receiver == 4) or (sender == 8 and receiver == 4):
        route = [6, 0, 3, 4]
    elif (sender == 7 and receiver == 5) or (sender == 8 and receiver == 5):
        route = [6, 0, 3, 5]
    elif sender == 1 and receiver == 2:
        route = [0, 2]
    elif sender == 2 and receiver == 1:
        route = [0, 1]
    elif sender == 4 and receiver == 5:
        route = [3, 5]
    elif sender == 5 and receiver == 4:
        route = [3, 4]
    elif sender == 7 and receiver == 8:
        route = [6, 8]
    elif sender == 8 and receiver == 7:
        route = [6, 7]
    else:
        route = []
    return route

class lock(object):
    def __init__(self, **kwargs):
        self.__dict__.update(kwargs)

def find_and_lock(earliest, slot_size, locks, route, route_full):
    if not route:
        return earliest
    #print("earliest", earliest)
    slot = find_slot(earliest, slot_size, route, locks)
    #print("slot", slot)
    lock_slot(slot, slot_size, route_full, locks)
    return slot;

def find_slot(instant, duration, route, locks):
    #print("route", route)
    if not route:
      lck = lock(set_ = instant, release = instant + duration, resource = 0)
    else:
      lck = lock(set_ = instant, release = instant + duration, resource = route[0])
      route.pop(0)
    if not route:
        return instant
    flag = False
    #print("number of locks", len(locks))
    while True:
        for i in range(len(locks)):
            if lck.resource == locks[i].resource:
                if lck.set_ < locks[i].release and locks[i].set_ < lck.release:
                    #print("FINALLY")
                    lck.set_ = lck.set_ + 1
                    lck.release = lck.release + 1
                    flag = True
                else:
                    flag = False
        if flag == True:
            continue
        slot = find_slot(lck.release, duration, route, locks)
        if slot == lck.release:
            break
        else:
            lck.set_ = slot
            lck.release = lck.set_ + duration
    return lck.set_

def lock_slot(slot, slot_size, route, locks):
    lck_set = slot
    for node in route:
        lck = lock(set_ = lck_set, release = lck_set + slot_size, resource = node)
        locks.append(lck)
        lck_set += slot_size

predictions = pd.read_csv('predictions_priorities.csv', header = None)
predictions = predictions.to_numpy()

end_systems = [1,2,4,5,7,8]
start_time = [0] * 10
end_time = [0] * 10
processor = [0] * 10

for i in range (4000):
    finalTable[i+16000].insert(7, "start_time", start_time, True)
    finalTable[i+16000].insert(8, "end_time", end_time, True)
    finalTable[i+16000].insert(9, "processor", processor, True)

messages_1 = messages9[0:400]
messages_2 = messages9[400:800]
messages_3 = messages9[800:1200]
messages_4 = messages9[1200:1600]
messages_5 = messages9[1600:]
messages_6 = messages10[0:400]
messages_7 = messages10[400:800]
messages_8 = messages10[800:1200]
messages_9 = messages10[1200:1600]
messages_10 = messages10[1600:]

jobs_1 = finalTable[16000:16400]
jobs_2 = finalTable[16400:16800]
jobs_3 = finalTable[16800:17200]
jobs_4 = finalTable[17200:17600]
jobs_5 = finalTable[17600:18000]
jobs_6 = finalTable[18000:18400]
jobs_7 = finalTable[18400:18800]
jobs_8 = finalTable[18800:19200]
jobs_9 = finalTable[19200:19600]
jobs_10 = finalTable[19600:]

predictions1 = predictions[0:400,:]
predictions2 = predictions[400:800,:]
predictions3 = predictions[800:1200,:]
predictions4 = predictions[1200:1600,:]
predictions5 = predictions[1600:2000,:]
predictions6 = predictions[2000:2400,:]
predictions7 = predictions[2400:2800,:]
predictions8 = predictions[2800:3200,:]
predictions9 = predictions[3200:3600,:]
predictions10 = predictions[3600:,:]


messages1 = messages9 + messages10
makespan_GA = makespan_GA9 + makespan_GA10
jobs1 = finalTable[16000:]
#Allocation
print("Allocation")
start = time.time()
with concurrent.futures.ProcessPoolExecutor() as executer:
       final1 = executer.submit(allocation,predictions1,jobs_1,messages_1,end_systems, 400)
       final2 = executer.submit(allocation,predictions2,jobs_2,messages_2,end_systems, 400)
       final3 = executer.submit(allocation,predictions3,jobs_3,messages_3,end_systems, 400)
       final4 = executer.submit(allocation,predictions4,jobs_4,messages_4,end_systems, 400)
       final5 = executer.submit(allocation,predictions5,jobs_5,messages_5,end_systems, 400)
       final6 = executer.submit(allocation,predictions6,jobs_6,messages_6,end_systems, 400)
       final7 = executer.submit(allocation,predictions7,jobs_7,messages_7,end_systems, 400)
       final8 = executer.submit(allocation,predictions8,jobs_8,messages_8,end_systems, 400)
       final9 = executer.submit(allocation,predictions9,jobs_9,messages_9,end_systems, 400)
       final10 = executer.submit(allocation,predictions10,jobs_10,messages_10,end_systems, 400)
end = time.time()
print(f'\n time: {end - start :.2f}s\n')
final_1, makespan_1 = final1.result()
final_2, makespan_2  = final2.result()
final_3, makespan_3  = final3.result()
final_4, makespan_4  = final4.result()
final_5, makespan_5  = final5.result()
final_6, makespan_6  = final6.result()
final_7, makespan_7  = final7.result()
final_8, makespan_8  = final8.result()
final_9, makespan_9  = final9.result()
final_10, makespan_10  = final10.result()

print("Done")
#print(jobs1[0])
#print(messages1[0])

#final1, makespan1 = allocation(predictions, jobs1, messages1, end_systems, 10)

total_makespan = makespan_1 + makespan_2 + makespan_3 + makespan_4 + makespan_5 + makespan_6 + makespan_7 + makespan_8 + makespan_9 + makespan_10

print("Mean makespan:",mean(total_makespan))
print("Mean makespan GA:",mean(makespan_GA))
pickle.dump(total_makespan, open("position_makespan_10.p", 'wb'))
pickle.dump(makespan_GA, open("GA_makespan_10.p", 'wb'))
