import click
import os
from collections import defaultdict

PHENOTOOLS = "../phenotools"

## check if exists and if not run

categories = defaultdict(str);
categories['ABNORMAL_CELLULAR'] = "HP:0025354";
categories['BLOOD'] = "HP:0001871";
categories['CONNECTIVE_TISSUE'] = "HP:0003549";
categories['HEAD_AND_NECK'] = "HP:0000152";
categories['LIMBS'] = "HP:0040064";
categories['METABOLISM'] = "HP:0001939";
categories['PRENATAL'] = "HP:0001197";
categories['BREAST'] = "HP:0000769";
categories['CARDIOVASCULAR'] = "HP:0001626";
categories['DIGESTIVE'] = "HP:0025031";
categories['EAR'] = "HP:0000598";
categories['ENDOCRINE'] = "HP:0000818";
categories['EYE'] = "HP:0000478";
categories['GENITOURINARY'] = "HP:0000119";
categories['IMMUNOLOGY'] = "HP:0002715";
categories['INTEGUMENT'] = "HP:0001574";
categories['MUSCLE'] = "HP:0003011";
categories['NERVOUS_SYSTEM'] = "HP:0000707";
categories['RESPIRATORY'] = "HP:0002086";
categories['SKELETAL'] = "HP:0000924";
categories['THORACIC_CAVITY'] = "HP:0045027";
categories['VOICE'] = "HP:0001608";
categories['CONSTITUTIONAL'] = "HP:0025142";
categories['GROWTH'] = "HP:0001507";
categories['NEOPLASM'] = "HP:0002664";




def check_phenotools():
    """
    Run the setup command to install phenotools if necessary
    :return:
    """
    if not os.path.exists(PHENOTOOLS):
        os.system('../setup.py')
    else:
        print("[INFO] we found ")

def run_phenotools(date, term):
    """
    Run phenotools for a single category
    :param categ:
    :return:
    """
    mycommand = "../phenotools hpo --hp ../hp.json --date %s --term %s --out tmp.txt" % (date, term)
    print(mycommand)
    os.system(mycommand)
    subontologyId = None
    subontologyName = None
    createdAfter = None
    total = None
    with open("tmp.txt") as f:
        for line in f:
            if line.startswith("#Subontology"):
                myline = line[13:].strip()
                subontologyId = myline[:11]
                myline = myline[12:]
                subontologyName = myline.replace(")","").strip()
            elif line.startswith("#Created after"):
                field = line.rstrip('\n').split(':')[1].strip()
                createdAfter = int(field)
            elif line.startswith("#Total"):
                field = line.rstrip('\n').split(':')[1].strip()
                total = int(field)
    if subontologyId is None:
        raise ValueError("Could not find subontologyId")
    elif subontologyName is None:
        raise ValueError("Could not find subontologyName")
    elif createdAfter is None:
        raise ValueError("Could not find createdAfter")
    elif total is None:
        raise ValueError("Could not find total")
    return subontologyId, subontologyName, createdAfter, total


def run_phenotools_annotations(date, enddate,term, annot):
    """
    Run phenotools for a single category
    :param categ:
    :return:
    """
    mycommand = "../phenotools annotation --hp ../hp.json -a %s --date %s --enddate %s --term %s --out tmp.txt" % (annot, date, enddate, term)
    print(mycommand)
    os.system(mycommand)
    subontologyId = None
    subontologyName = None
    createdAfter = None
    total = None
    with open("tmp.txt") as f:
        for line in f:
            print(line)
            if line.startswith('#total annotations to'):
                field = line.rstrip('\n').split(":")[1]
                total = int(field)
            elif line.startswith("#total annotations newer"):
                field=line.rstrip('\n').split(":")[1]
                createdAfter = int(field)
    if total is None:
        raise ValueError("could not parse total")
    elif createdAfter is None:
        raise ValueError("could not parse createdAfter")
    return createdAfter, total


def run_phenotools_hpo():
    fh = open('hpocounts.txt', 'wt')
    fh.write("#Terms\n")
    fh.write("#start-date:%s\n" % date)
    fh.write("#end-date:%s\n" % date)
    fh.write("subontology.id\tsubontology.label\tcreated.after\ttotal\n")
    for k, v in categories.items():
        subontologyId, subontologyName, createdAfter, total = run_phenotools(date, v)
        fh.write("%s\t%s\t%s\t%s\n" % (subontologyId, subontologyName, createdAfter, total))
    fh.close()

def run_annotations(startdate, enddate, annotfile):
    gh = open('annotcounts.txt', 'wt')
    gh.write("#Annotations\n")
    gh.write("#start-date:%s\n" % startdate)
    gh.write("#end-date:%s\n" % enddate)
    gh.write("subontology.id\tsubontology.label\tcreated.after\ttotal\n")
    for k, v in categories.items():
        createdAfter, total = run_phenotools_annotations(startdate, enddate, v, annotfile)
        gh.write("%s\t%s\t%s\t%s\n" % (v, k, createdAfter, total))
    gh.close()


@click.command()
@click.option('--date', '-d')
@click.option('--enddate', '-e')
@click.option('--annot', '-a')
@click.option('--hpo', '-h')
def main(date, enddate, hpo, annot):
    if date is None or len(date)==0:
        print("Error --date argument required")
        return
    if hpo is None or len(hpo)==0:
        print("Error --hpo argument required")
        return
    if annot is None or len(annot) == 0:
        print("Error --annot argument required")
        return
    check_phenotools()
    run_annotations(startdate=date, enddate=enddate, annotfile=annot)
 
    
if __name__ == "__main__":
    main()