
# coding: utf-8

# In[2]:

import pandas as pd
import numpy as np
from pandas import Series, DataFrame
from io import StringIO
import matplotlib.pyplot as plt
get_ipython().magic('matplotlib inline')


# In[3]:

def read_from_file(filename) :
    f = open(filename, 'r')
    lines = f.readlines()

    #appnames = []
    contents = []
    log = ''
    capture = False

    for line in lines:
        #if(line.isspace()): continue

        if(('AUL Launching' in line or 'Fork Launching' in line) and 'tv-viewer' not in line and 'killall' not in line):
            capture = True
            appname = line.split('\"')[1]
            
        if(capture == True and ('app_id' in line or 'launch app' in line or 'spend time for' in line or 'not found' in line)) :
            continue

        if(capture == True and 'stablised' in line):
            sec = float(line.split(' ')[-1][0:-4])
            contents.append({'appname': appname, 'log': log, 'sec': sec})
            log = ''
            capture = False
            
        if(capture is True):
            log += line
            
    f.close()
    return contents


# In[1]:

def draw_cpu_chart(content, ax, xmax):
    frame = pd.read_csv(StringIO(content['log']), sep='\s+', skiprows=[0, 1])
    title = content['appname'] + ' ' + str(content['sec'])
    ax = frame[['tot.cpu', 'usr.cpu', 'kswapd']].plot(ax=ax, 
                                                      x=frame['time'], 
                                                      title=title, 
                                                      linewidth=2, 
                                                      ylim=(0, 105),
                                                      xlim=(0, xmax),
                                                      grid=True, 
                                                      color=['b','g','r'],
                                                      legend=False)
    ax.annotate("launching finish",
                (content['sec'], 100),
                 xytext=(5, -15), 
                 textcoords='offset points')
    ymin, ymax = ax.get_ylim()
    ax.vlines(x=content['sec'], ymin=ymin, ymax=ymax-1, color='k', linewidth=2, linestyle='dashed')
    ax.set_xlabel("time (s)")
    ax.set_ylabel("cpu usage (%)")


# In[4]:

def draw_mem_chart(content, ax, xmax):
    frame = pd.read_csv(StringIO(content['log']), sep='\s+', skiprows=[0, 1])
    title = content['appname'] + ' ' + str(content['sec'])
    ax = frame[['mem.av', 'mem.fr']].plot(ax=ax, 
                                                      x=frame['time'], 
                                                      title=title, 
                                                      linewidth=2, 
                                                      xlim=(0, xmax),
                                                      grid=True,
                                                      legend=True)
    ax.annotate("launching finish",
                (content['sec'], 100),
                 xytext=(5, -15), 
                 textcoords='offset points')
    ymin, ymax = ax.get_ylim()
    ax.vlines(x=content['sec'], ymin=ymin, ymax=ymax-1, color='k', linewidth=2, linestyle='dashed')
    ax.set_xlabel("time (s)")
    ax.set_ylabel("memory (MB)")


# In[5]:

def show(filename):
    contents = read_from_file(filename)
    
    fig, axes = plt.subplots(len(contents), 2, figsize=(16, 20))
    fig.subplots_adjust(hspace=0.7)

    for i, content in enumerate(contents):
        xmax = content['sec']+ 1.0
        draw_cpu_chart(content, axes[i][0], xmax)
        draw_mem_chart(content, axes[i][1], xmax)


# In[6]:

def launching_time(filename):
    results = []
    contents = read_from_file(filename)
    for content in contents:
        results.append(content['sec'])
    return results


# In[7]:

def compare_all(filename1, filename2):
    contents1 = read_from_file(filename1)
    contents2 = read_from_file(filename2)
    
    number_of_chart = len(contents1) + len(contents2)
    
    fig, axes = plt.subplots(number_of_chart, 2, figsize=(16, number_of_chart * 2))
    fig.subplots_adjust(hspace=0.7)
    
    for i, (content1, content2) in enumerate(zip(contents1, contents2)):
        xmax = max(content1['sec'], content2['sec']) + 1.0
        draw_cpu_chart(content1, axes[2*i][0], xmax)
        draw_mem_chart(content1, axes[2*i][1], xmax)
        draw_cpu_chart(content2, axes[2*i+1][0], xmax)
        draw_mem_chart(content2, axes[2*i+1][1], xmax)


# In[8]:

def compare(filename1, filename2, select):
    contents1 = read_from_file(filename1)
    contents2 = read_from_file(filename2)
    
    number_of_chart = 2
    fig, axes = plt.subplots(number_of_chart, 1, figsize=(12, number_of_chart * 2), sharex=True)
    fig.subplots_adjust(hspace=0.7)
    
    for i, (content1, content2) in enumerate(zip(contents1, contents2)):
        if(select != None and select == i):
            xmax = max(content1['sec'], content2['sec']) + 1.0
            draw_single_chart(content1, axes[0], xmax)
            draw_single_chart(content2, axes[1], xmax)


# In[9]:

def show_result(files, cols):
    frames = []
    for file in files :
        frame = DataFrame(launching_time(file))
        frame = frame.T
        frame.columns = cols
        frame['file'] = file
        frames.append(frame)

    df = pd.concat(frames, ignore_index=True)
    df['sum'] = df.sum(axis=1)
    return df


# In[ ]:



