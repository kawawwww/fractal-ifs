# -*- coding: utf-8 -*-
"""
Created on Sat Aug 05 23:55:12 2017
@author: Kazushige Okayasu, Hirokatsu Kataoka
"""
import math
import random
import numpy as np
from PIL import Image
import matplotlib.pyplot as plt

class ifs_function():
    #prev_x, prev_yが引数になる
	def __init__(self, prev_x, prev_y):
		# previous (x, y)
		self.prev_x,self.prev_y = prev_x,prev_y
		# IFS function
		self.function  = []
		# Iterative results
		self.xs,self.ys = [],[]
		# Add initial value
		self.xs.append(prev_x),self.ys.append(prev_y)
		# Select function
		self.select_function = []
		# Calculate select function
		self.temp_proba = 0.0

	def set_param(self,a,b,c,d,e,f,proba,**kwargs):
		# 初期パラメータと選択機能
		temp_function  = {"a":a,"b":b,"c":c,"d":d,"e":e,"f":f,"proba":proba} 
        #要素を追加
		self.function.append(temp_function)
		# 関数が追加されたときのプラス確率       
		self.temp_proba += proba
		self.select_function.append(self.temp_proba)

	def calculate(self,iteration):
		# ランダムシードを修正
        #randomseed:コンピューターが乱数シーケンスを生成するときの開始点を指定
		rand = np.random.random(iteration)
		select_function = self.select_function
		function = self.function
        #三点追加
		#ax,ay=self.ax,self.ay
		#bx,by=self.bx,self.by
		#cx,cy=self.cx,self.cy
		prev_x,prev_y = self.prev_x, self.prev_y
        ##############################
        ###ここでシフトや回転をする###
        ##############################
		#for i in xrange(iteration-1): #python2.x
		for i in range(iteration-1):   #python3.x
			for j in range(len(select_function)):
				if rand[i] <= select_function[j]:
                    #回転シフト
					next_x = prev_x * function[j]["a"] + prev_y * function[j]["b"] + function[j]["e"]
					next_y = prev_x * function[j]["c"] + prev_y * function[j]["d"] + function[j]["f"]
					break
                    #if rand[i] <= select_function[j]:を満たしたらbreak(中断)
			self.xs.append(next_x),self.ys.append(next_y)
			prev_x = next_x
			prev_y = next_y

	# Inner function
	def __rescale(self,image_x,image_y,pad_x,pad_y):
		# スケール調整
		xs = np.array(self.xs)
		ys = np.array(self.ys)
        #値が入っていないものを消去
        #np.any()配列内に1つでも条件を満たす要素があるかどうかを確認
        #身長が入るべき列に値が入っていないようなこともあります。このように値が欠けている状態を「欠損値」と呼び,np.isnanで探し出す
		if np.any(np.isnan(xs)):
			#print("x is nan")
            #np.where;joukenn(np.isnan)のインデックスが抽出
			nan_index = np.where(np.isnan(xs))
            
            ##########
            ###拡張###
            ##########
            #####################range(開始、終了）
			extend = np.array(range(nan_index[0][0]-100,nan_index[0][0]))
            #extendにnanindex追加
			delete_row = np.append(extend,nan_index)
            #xs要素から削除、次の引数で要素のインデックスを指定、axis0;横軸指定
			xs = np.delete(xs,delete_row,axis=0)
			ys = np.delete(ys,delete_row,axis=0)
			#print ("early_stop: %d" % len(xs))
		if np.any(np.isnan(ys)):
			#print("y is nan")
			nan_index = np.where(np.isnan(ys))
			extend = np.array(range(nan_index[0][0]-100,nan_index[0][0]))
			delete_row = np.append(extend,nan_index)
			xs = np.delete(xs,delete_row,axis=0)
			ys = np.delete(ys,delete_row,axis=0)
			#print ("early_stop: %d" % len(ys))
        #minが０未満だったら
        #0以下がなくなる
		if np.min(xs) < 0.0:
			xs -= np.min(xs)
		if np.min(ys) < 0.0:
			ys -= np.min(ys)
       #最大値最小値を抽出
		xmax,xmin,ymax,ymin = np.max(xs),np.min(xs),np.max(ys),np.min(ys)
        #16bit符号なし整数
        #xs/(xmax-cmin)=0~1の値に
        ######pad_x????
		self.xs = np.uint16(xs / (xmax-xmin) * float(image_x-2*pad_x)+float(pad_x))
		self.ys = np.uint16(ys / (ymax-ymin) * float(image_y-2*pad_y)+float(pad_y))
		#self.xs = np.uint8(xs / (xmax-xmin) * float(image_x-2*pad_x)+float(pad_x))
		#self.ys = np.uint8(ys / (ymax-ymin) * float(image_y-2*pad_y)+float(pad_y))
        
	def draw_point(self,image_x,image_y,pad_x,pad_y,set_color,count):
		self.__rescale(image_x,image_y,pad_x,pad_y)
        #image.new;配列から画像データにする。引数は（mode,size,color）
		image = np.array(Image.new("RGB", (image_x, image_y)))

		img=np.zeros((512,512))

          
		for i in range(len(self.xs)):   #python3.x
			img[self.ys[i],self.xs[i]] +=1
            #img[self.ys[i],self.xs[i]] +=5
		vmax=np.max(img)
		img=img/vmax*255
		for i in range(512):
			for j in range(512):
				
				#print(img)            
				
				image[i,j,:]=img[i,j].astype("uint8"),img[i,j].astype("uint8"),img[i,j].astype("uint8")

		return image