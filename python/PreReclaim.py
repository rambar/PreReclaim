
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

def draw_cpu_chart(df, title, finish, ax, xmax):
    ax = df[['tot.cpu', 'usr.cpu', 'kswapd', 'rescd']].plot(
        ax=ax, 
        title=title, 
        linewidth=1, 
        ylim=(0, 105),
        xlim=(0, xmax),
        grid=True, 
        style=['b-','g-','r-', 'k-'],
        legend=False)
    
    ax.legend(loc='upper center', bbox_to_anchor=(0.5, -0.1),
          fancybox=True, shadow=True, ncol=4)

    #ax.annotate("launching finish",
    #            (content['sec'], 100),
    #             xytext=(5, -15), 
    #             textcoords='offset points')
    ymin, ymax = ax.get_ylim()
    ax.vlines(x=finish, ymin=ymin, ymax=ymax-1, color='y', linewidth=2)
    ax.set_xlabel("time (s)")
    ax.set_ylabel("cpu usage (%)")

# In[4]:

def draw_mem_chart(df, title, finish, ax, xmax):
    ax = df[['available', 'free']].plot(
        ax=ax, 
        title=title, 
        linewidth=1, 
        xlim=(0, xmax),
        style=['b-','g-'],
        grid=True,
        legend=False)
    
    ax.legend(loc='upper center', bbox_to_anchor=(0.5, -0.1),
          fancybox=True, shadow=True, ncol=2)

    ymin, ymax = ax.get_ylim()
    ax.vlines(x=finish, ymin=ymin, ymax=ymax-1, color='y', linewidth=2)
    ax.set_xlabel("time (s)")
    ax.set_ylabel("memory (MB)")

def draw_mem_chart2(df, title, finish, ax, xmax):    
    ax = df[['swap', 'pagecache']].plot(
        ax=ax, 
        title=title, 
        linewidth=1, 
        xlim=(0, xmax),
        style=['b--', 'g-'],
        grid=True,
        secondary_y=['pagecache'],
        legend=True)
    
    lines = ax.get_lines() + ax.right_ax.get_lines()
    
    ax.legend(lines, [l.get_label() for l in lines], loc='upper center', bbox_to_anchor=(0.5, -0.1),
          fancybox=True, shadow=True, ncol=3)

    ymin, ymax = ax.get_ylim()
    ax.vlines(x=finish, ymin=ymin, ymax=ymax-0.1, color='y', linewidth=2)
    ax.set_xlabel("time (s)")
    ax.set_ylabel("swap (MB)")
    ax.right_ax.set_ylabel("pagecache (MB)")

# In[5]:

def pre_process(content):
    df = pd.read_csv(StringIO(content['log']), sep='\s+', skiprows=[0, 1])
    df.set_index('time', inplace=True)
    
    if('mem.pc' not in list(df)):
        df['mem.pc'] = 0

    if('rescd' not in list(df)):
        df['rescd'] = 0
    
    df.rename(columns={'mem.av': 'available', 'mem.fr': 'free', 'mem.pc': 'pagecache', 'mem.sw': 'swap'}, inplace=True)
    title = content['appname'] + ' ' + str(content['sec'])
    finish = content['sec']
    
    return df, title, finish
    
def show(filename):
    contents = read_from_file(filename)
    
    rows = len(contents)
    
    fig, axes = plt.subplots(len(contents), 3, figsize=(16, rows * 4))
    fig.subplots_adjust(hspace=0.7)

    for i, content in enumerate(contents):
        df, title, finish = pre_process(content)
        xmax = finish + 1.0
        draw_cpu_chart(df, title, finish, axes[i][0], xmax)
        draw_mem_chart(df, title, finish, axes[i][1], xmax)
        draw_mem_chart2(df, title, finish, axes[i][2], xmax)

# In[6]:

# In[7]:

def compare_all(filename1, filename2):
    contents1 = read_from_file(filename1)
    contents2 = read_from_file(filename2)
    
    rows = len(contents1) * 2
    
    fig, axes = plt.subplots(rows, 3, figsize=(16, rows * 4))
    fig.subplots_adjust(hspace=0.7)
    
    for i, (content1, content2) in enumerate(zip(contents1, contents2)):
        xmax = max(content1['sec'], content2['sec']) + 1.0
        
        df, title, finish = pre_process(content1)
        draw_cpu_chart(df, title, finish , axes[2*i][0], xmax)
        draw_mem_chart(df, title, finish , axes[2*i][1], xmax)
        draw_mem_chart2(df, title, finish , axes[2*i][2], xmax)
        
        df, title, finish = pre_process(content2)
        draw_cpu_chart(df, title, finish , axes[2*i+1][0], xmax)
        draw_mem_chart(df, title, finish , axes[2*i+1][1], xmax)
        draw_mem_chart2(df, title, finish , axes[2*i+1][2], xmax)


# In[8]:

def compare(filenames, select):
    rows = len(filenames)
    fig, axes = plt.subplots(rows, 3, figsize=(16, rows * 4), sharex=True)
    fig.subplots_adjust(hspace=0.7)

    xmax = 0
    for filename in filenames:
        contents = read_from_file(filename)
        xmax = max(xmax, contents[select]['sec'])

    for i, filename in enumerate(filenames):
        contents = read_from_file(filename)
        
        df, title, finish = pre_process(contents[select])
    
        draw_cpu_chart (df, title, finish, axes[i][0], xmax + 1)
        draw_mem_chart (df, title, finish, axes[i][1], xmax + 1)
        draw_mem_chart2(df, title, finish, axes[i][2], xmax + 1)

# In[9]:

def show_result(files):
    frames = []
    for file in files :
        frame = launching_time(file)
        frame['file'] = file
        frames.append(frame)

    df = pd.concat(frames, ignore_index=True)
    df['sum'] = df.sum(axis=1)
    return df

def launching_time(filename):
    results = []
    names = []
    contents = read_from_file(filename)
    for content in contents:
        results.append(content['sec'])
        names.append(content['appname'])
    
    df = DataFrame(results)
    df = df.T
    df.columns = names 
    return df