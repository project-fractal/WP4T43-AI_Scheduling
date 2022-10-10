currpath="$(pwd)"
exe="range"
len=$(ls | grep  "xargs_script*" | wc -l)
#echo "$len"
for i in $( seq 1 $len )
do  
	ed xargs_script$i.sh < add_line.ed
	sed -i -e "s|/work/ws-tmp/cl603105-workspace/NewGA|$currpath|g" xargs_script$i.sh
	sed -i -e "s|range_restri|Inputs|g" xargs_script$i.sh
	sed -i -e "s|range_rest|$exe|g" xargs_script$i.sh
        #sed -i 's#/work/ws-tmp/cl603105-workspace/NewGA#$currpath#g' xargs_script$i.sh
	#sed -i 's/<old-input-file-name>/<new-input-file-name>/g' xargs_script$i.sh
done

