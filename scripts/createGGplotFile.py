# Run the phenotools toplevel on two files. Then use this file to make a ggplot file that will display a stacked bar plot
# of HPO categories
from collections import defaultdict

hpo1 = '../terms2018.txt'
hpo2 = '../terms2020.txt'


categories = defaultdict(str);
categories['Abnormal cellular'] = "HP:0025354";
categories['Blood'] = "HP:0001871";
categories['Connective tissue'] = "HP:0003549";
categories['Head and neck'] = "HP:0000152";
categories['Limbs'] = "HP:0040064";
categories['Metabolism/Lab'] = "HP:0001939";
categories['Prenatal'] = "HP:0001197";
categories['Breast'] = "HP:0000769";
categories['Cardiovascular'] = "HP:0001626";
categories['Digestive'] = "HP:0025031";
categories['Ear'] = "HP:0000598";
categories['Endocrine'] = "HP:0000818";
categories['Eye'] = "HP:0000478";
categories['Genitourinary'] = "HP:0000119";
categories['Immunology'] = "HP:0002715";
categories['Integument'] = "HP:0001574";
categories['Muscle'] = "HP:0003011";
categories['Nervous system'] = "HP:0000707";
categories['Respiratory'] = "HP:0002086";
categories['Skeletal'] = "HP:0000924";
categories['Thoracic cavity'] = "HP:0045027";
categories['Voice'] = "HP:0001608";
categories['Constitutional'] = "HP:0025142";
categories['Growth'] = "HP:0001507";
categories['Neoplasm'] = "HP:0002664";

id2cat = defaultdict(str)
for k, v in categories.items():
    id2cat[v] = k

def input_category_counts(fname):
    ctdict = defaultdict(int)
    with open(fname) as f:
        for line in f:
            category = line.rstrip().split('\t')[1]
            ctdict[category] += 1
    return ctdict


ct_hp1 = input_category_counts(hpo1)
ct_hp2 = input_category_counts(hpo2)



fh = open("count_barplot.R", "wt")
fh.write("library(ggplot2)\n")
fh.write("theme_set(theme_bw())\n")

N = 2 # two year-phaseslen(id2cat)
fh.write("categories <-c(")
rep_list = []
for k, v in categories.items():
    rep_list.append('rep("%s", %d)' %(k, N))
fh.write(",".join(rep_list))
fh.write(")\n")
fh.write('years <- factor(rep(c("2008-2018", "2018-2020"),%d))\n' % len(id2cat))
fh.write('years <- relevel(years, "2018-2020")\n')
value_list = []
for k, v in categories.items():
    hpoid = v
    hpo1 = ct_hp1[v]
    hpo2 = ct_hp2[v] - hpo1
    value_list.append(hpo1)
    value_list.append(hpo2)
    
    
fh.write("value <- c(")
fh.write(','.join([str(i) for i in value_list]))
fh.write(")\n")
fh.write('data <- data.frame(categories, years, value)\n')
fh.write("mycolors <- c( '#3c5488', '#e64b35')\n")
fh.write('g <- ggplot(data, aes(fill=years, y=value, x=reorder(categories, -value),group=row.names(years))) + \n')
fh.write('    geom_bar(position="stack", stat="identity") + geom_col()\n')
fh.write('g <- g + coord_flip() + scale_fill_manual(values=mycolors)\n')
fh.write('g <- g + theme(axis.title=element_blank(),\n' +
               'axis.text.y = element_text(size = 12),\n' +
               'axis.text.x = element_text(size = 14), legend.position=c(0.75,0.925), legend.title=element_blank(), legend.text=element_text(size=24))\n')
fh.write('ggsave("hpo_bp.pdf")\n')
fh.close()