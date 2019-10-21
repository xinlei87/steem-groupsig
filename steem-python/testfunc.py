import random
import string
import steembase
import struct
import steem
from time import sleep
from time import time
from steem.transactionbuilder import TransactionBuilder
from steembase import operations
from steembase.transactions import SignedTransaction
from resultthread import MyThread
from charm.toolbox.pairinggroup import PairingGroup,ZR,G1,G2,GT,pair
from charm.toolbox.secretutil import SecretUtil
class GroupSignature():

    def __init__(self, groupObj):
        global util, group
        util = SecretUtil(groupObj, debug)
        self.group = groupObj

    def pkGen(self,h1str):
        gstr = "[6172776968119684165170291368128433652817636448173749093457023424948260385279837018774774149930982188956916913145008943931711059687988096415181819433817738, 8687587692191287108886119971783525001480020593934954052605681527814232399216375005546606067382536684351686344089456732201641997200939472924879001214689004]"
        g2str= "[7648994551207171188393784904797547917038803147671542540175090956205316897431443264058433935237605598252399113847934759009659621851760599508222321653067284, 922489308494109901795721463782161260386164061515796674638135394871842997698175772871045949554746517321480649326465484116060959631197509151923296896589720]"
        u0str = "[180015966842918451436547451263180245588308971597733548673037049536176684754209695288737508087729924028686259002375511049961436438196866049956546630518033, 1295050197915669955783867959538729894307963685491173858450359845766785488725907727220684060845012524740394664162328817669422178637925195059862486690053923]"
        u1str = "[2555472719769037960206282327195096320915753855199743796256065902544200822503613205017219993060986152240852358189992579821797745072366030183800897743028220, 7573705235093543416041007636313631591000596820214067724084077929638801811700093589294454562385664531190678890366928407286293582994146887505184778221562373]"
        u2str = "[6876276970903121931083294698771200898345396507892092532649392211995185517437159402176975528760594250374462299539306423347676182899798006533425047523984724, 5323739238507219125881988073888745575030677585404965990610324901624530474522642705344792075909082041735695801098770187248023797265998906693745587936574078]"
        u3str = "[6628726193389375981104409894060310698729022957801238449570622103067828518416602275957863668289683360250722835022304456841105526036470008237775051984811323, 862537748555943361001122447731987661405436458862545177179548603003392540530328380518694788420155531238391922289886044667763424887444361610972254938158280]"
        u4str = "[8157254219580822599577995921928211211847392705248772673869189421041858895589817404931780741226510985762564598862965174380020566416411083236239871342674775, 4736677719200783513058679582227494204159737596114643136852532046080608159561620208171676599501713934575216178076006396924589443776642926902969084668055006]"
        hstr = "[6248393417805371388321299785844751688345516419281230263497475615452026459314582553252281068616984105757749673095320346188725995701858182333525688832492249, 351368339412205819108519989143352052898751906937356995136442397753142226531384069336237369861919799955237545207977716196031001184146017796598836939617335]"
        nstr = "[75201312764006187596691102237923705656296213254701583615255122742135170369075831428394751330697143847448434841509551532135632624530360013837581615049543, 3886258599652934715331576083899336629981754505948456216299528998628273512432828729344158706718479567056972375128622026273382126529171409058157562418608963]"

        g = self.group.fromstr(gstr,10, G1)
        g2 = self.group.fromstr(g2str,10, G2)
        u0 = self.group.fromstr(u0str, 10 , G2)
        u1 = self.group.fromstr(u1str, 10 , G2)
        u2 = self.group.fromstr(u2str, 10 , G2)
        u3 = self.group.fromstr(u3str, 10 , G2)
        u4 = self.group.fromstr(u4str, 10 , G2)
        h = self.group.fromstr(hstr, 10 , G1)
        n = self.group.fromstr(nstr, 10 , GT)
        h1 = self.group.fromstr(h1str, 10 , G1)

        pk={'g':g, 'g2':g2, 'u0':u0,'u1':u1,'u2':u2,'u3':u3,'u4':u4,'h':h,'n':n,'h1':h1}
        
        return pk

    def uskGen(self,usklist, pk, GID, UID, L,k):
        t1 = time()
        b0 = self.group.gen1_0(1)
        b3 = self.group.gen1_0(1)
        b4 = self.group.gen1_0(1)
        b5 = self.group.gen1_0(1)
        
        r2 = self.group.random(ZR)

        for i in range(k):
            b0 = b0*(usklist[i]['b0']**L[i])
            b3 = b3*(usklist[i]['b3']**L[i])
            b4 = b4*(usklist[i]['b4']**L[i])
            b5 = b5*(usklist[i]['b5']**L[i])
        
        b0 = b0 * (pk['u0'] * (pk['u1'] ** GID) * (pk['u2'] ** UID))**r2
        b3 = b3 *(pk['u3']**r2)
        b4 = b4 * (pk['u4']**r2)
        b5 = b5 * (pk['g'] ** r2)

        usk = {'b0':b0,'b3':b3,'b4':b4,'b5':b5}
        t2 = time()
        with open("extracttime.txt", 'a') as f:
            f.write(str(t2-t1))
            f.write('\n')
        return usk

    def LGen(self,n,k):
        L = []
        I = self.group.random(ZR)
        J = self.group.random(ZR)
        for i in range(n):
            L.append(self.group.random(ZR))
            L[i].set(1)
            I.set(i+1)
            for j in range(1,k+1):
                print(j)
                J.set(j)
                if (i+1) != j:
                    L[i]=L[i]*((J)/(J-I))
        return L

    def verifyUsk(self,usk,vk,pk, GID, UID):
        g = pk['g']
        g2 = pk['g2']
        u0 = pk['u0']
        u1 = pk['u1']
        u2 = pk['u2']
        u3 = pk['u3']
        u4 = pk['u4']

        b0 = usk['b0']
        b5 = usk['b5']
        b3 = usk['b3']
        b4 = usk['b4']

        return  pair(g,b0) == (pair(vk,g2) * pair(b5,u0) * pair(b5,u1**GID) * pair(b5,u2**UID)) and pair(g,b3)==pair(b5,u3) and pair(g,b4)==pair(b5,u4)

    def sign(self, title,usk,pk,GID,UID,groupID):
        t1 = time()
        m = self.group.hash(title)
        b0 = usk['b0']
        b3 = usk['b3']
        b4 = usk['b4']
        b5 = usk['b5']
        
        r4 = self.group.random(ZR)
        r3 = self.group.random(ZR)
        k = self.group.random(ZR)

        c0 = b0*(b3**m)*(b4**r4)*((pk['u0']*(pk['u1']**GID)*(pk['u2']**UID)*(pk['u3']**m)*(pk['u4']**r4))**r3)
        c5 = b5*(pk['g']**r3)
        c6 = (pk['u2']**UID)*(pk['u4']**r4)
        e1 = pk['g']**k
        e2 = (pk['u0']*(pk['u1']**GID))**k
        e3 = (pk['n']**UID)*(pair(pk['h1'],pk['g2'])**k)

# 产生pok
        f = pk['u0']* (pk['u1']**GID)
        gp = pair(pk['h1'],pk['g2'])

        k1 = self.group.random(ZR)
        k2 = self.group.random(ZR)
        k3 = self.group.random(ZR)
        
        r1 = (pk['u2']**k1)*(pk['u4']**k2)
        r2 = pk['g']**k3
        r3 = f**k3
        t4 = (pk['n']**k1) * (gp**k3)

        hashstr = str(r1) + str(r2) + str(r3) + str(t4) 

        c = self.group.hash(hashstr)
        
        s1 = k1 + c * UID

        s2 = k2 + c * r4 

        s3 = k3 + c * k

        signature = {'c0':c0,'c5':c5,'c6':c6,'e1':e1,'e2':e2,'e3':e3,'c':c,'s1':s1,'s2':s2,'s3':s3}
        t2 = time()
        with open("vssigntime.txt", 'a') as f:
            f.write(str(t2-t1))
            f.write('\n')
        print("gs time", t2-t1)
        return signature
    
    def open(self,okliststr,L,k):
        
        oklist =[]
        for ok in okliststr:
            oklist.append({'ok1':self.group.fromstr(ok['ok1'], 10, GT),'ok2':self.group.fromstr(ok['ok2'], 10, GT)})
        ok1 = self.group.gen1_0(1)
        ok2 = self.group.gen1_0(1)
        for i in range (k):
            ok1 = ok1 * (oklist[i]['ok1']**L[i])
            ok2 = ok2 * (oklist[i]['ok2']**L[i])
        
        return ok1/ok2


def get_usk(userID ,GID ,UID, h1str = "", count = 0):

    pk = {}
    for i in range(n):
        vkliststr.append(clientlist[i].get_vk()['vk'])
        vklist.append(group_signature.group.fromstr(vkliststr[i], 10, G1))

        uskliststr.append(clientlist[i].user_extract(userID))
        usklist.append({})
        usklist[i]['b0'] = group_signature.group.fromstr(uskliststr[i]['b0'], 10, G2)
        usklist[i]['b3'] = group_signature.group.fromstr(uskliststr[i]['b3'], 10, G2)
        usklist[i]['b4'] = group_signature.group.fromstr(uskliststr[i]['b4'], 10, G2)
        usklist[i]['b5'] = group_signature.group.fromstr(uskliststr[i]['b5'], 10, G1)
        print(usklist[i])
        if h1str == "" or h1str == "0" or h1str == 0:
            h1str = clientlist[i].get_pk()['pk']
            print("h1str",h1str)
            pk = group_signature.pkGen(h1str)
            print("pk---------------\n", pk)

        if (group_signature.verifyUsk(usklist[i], vklist[i], pk, GID, UID)):
            count = count + 1
        else:
            print("key is invalide\n\n")
    usk = group_signature.uskGen(usklist, pk, GID, UID, L, k)

    print("usk---------------\n",usk)
    return pk, usk

def get_lam(sig):
    okliststr = []
    i = 0
    for client in clientlist:
        okstr = client.get_ok(str(sig['e1']), str(sig['e2']))
        print(okstr)
        okliststr.append(okstr)
        i = i + 1

    if i < k:
        print("the number of ok is not enough\n")
        return

    lam = group_signature.open(okliststr, L, k)
    return lam

def tx_build_broad(op, steemd_instance, wallet_instance, account):
    tx = TransactionBuilder(steemd_instance=steemd_instance, wallet_instance=wallet_instance,
                            no_broadcast=False)
    tx.appendOps(op)
    tx.appendSigner(account, 'posting')
    tx.sign()
    # print("txsign",tx)
    re = tx.broadcast()
    return re

def tx_build(op, steemd_instance, wallet_instance, account):
    tx = TransactionBuilder(steemd_instance=steemd_instance, wallet_instance=wallet_instance,
                            no_broadcast=False)
    tx.appendOps(op)
    tx.appendSigner(account, 'posting')
    tx.sign()
    # print("txsign",tx)
    # re = tx.broadcast()
    return tx

def annoy_commit(account, usk, pk, GID, UID,title = "paper_title",  body = "paper_body", groupID="computer"):
    annoy_author = 'nya'
    # group signature   ------title 必须   这里面是对title进行hash 然后使用usk对hash进行签名
    sig = group_signature.sign(title, usk, pk, GID, UID, groupID)

    permlink = ''.join(random.choices(string.digits, k=6))
    # permlink = "222223"
    print("permlink is " + permlink)
    op = operations.CommitPaper(
        **{
            "account": account,
            "author": annoy_author,
            "permlink": permlink,
            "title": title,
            "body": body,
            "json_metadata": "",
            "c0": str(sig['c0']),
            "c5": str(sig['c5']),
            "c6": str(sig['c6']),
            "e1": str(sig['e1']),
            "e2": str(sig['e2']),
            "e3": str(sig['e3']),
            "c": str(sig['c']),
            "s1": str(sig['s1']),
            "s2": str(sig['s2']),
            "s3": str(sig['s3'])
        }
    )
    print("commitop",op)
    return op, sig, permlink

def open_op(account, sig, userID, permlink):
    lam = get_lam(sig)
    #E = (pk['n'] ** UID) * lam  #计算出e3  即签名的e3 判断是否相等
    op = operations.ApplyOpen(
        **{
            'account': account,
            'author': userID,
            'lambda': str(lam),
            'permlink': permlink,
            'json_metadata': ""
        }
    )
    return op

def annoy_commit_tx(account, usk, pk, GID, UID, steemd_instance, wallet_instance, title = "paper_title",  body = "paper_body"):
    commitop, ssig, permlink = annoy_commit(account, usk, pk, GID, UID,title = "paper_title",  body = "paper_body", groupID="computer")
    re = tx_build_broad(commitop, steemd_instance, wallet_instance, account)
    print("commit-re", re)
    return ssig, permlink

def open_tx(account, ssig, userID, permlink, steemd_instance, wallet_instance ):
    openop = open_op(account, ssig, userID, permlink)
    re = tx_build_broad(openop, steemd_instance, wallet_instance, account)
    print("open-re", re)

#一个节点的 并发产生交易
def one_mul_annoy_tx(account, usk, pk, UID, steemd, wallet):
    ssiglistone=[]
    permlinklistone=[]
    threads = []
    for i in range(nodeTX):
        t = MyThread(annoy_commit_tx, args=(account, usk, pk, GID, UID, steemd, wallet))
        threads.append(t)
    for t in threads:
        t.start()
    for t in threads:
        t.join()
    for t in threads:
        ssig, permlink = t.get_result()
        ssiglistone.append(ssig)
        permlinklistone.append(permlink)
    return ssiglistone, permlinklistone

def one_mul_open_tx(account, ssiglistone, userID, permlinklistone, steemd , wallet):
    threads = []
    for i in range(nodeTX):
        t = MyThread(open_tx,
                     args=(account, ssiglistone[i], userID, permlinklistone[i], steemd, wallet))
        threads.append(t)
    for t in threads:
        t.start()
    for t in threads:
        t.join()

def mul_annoy_tx(usk, pk, UID):
    ssiglist=[]
    permlinklist=[]
    threads = []
    for i in range(n):
        # t = MyThread(annoy_commit_tx, args=(accountlist[i], usk, pk, GID, UID, clientlist[i].steemd, clientlist[i].wallet))
        t = MyThread(one_mul_annoy_tx,
                     args=(accountlist[i], usk, pk, UID, clientlist[i].steemd, clientlist[i].wallet))
        threads.append(t)
    for t in threads:
        t.start()
    for t in threads:
        t.join()
    for t in threads:
        ssig, permlink = t.get_result()
        ssiglist.append(ssig)
        permlinklist.append(permlink)
    return ssiglist, permlinklist

#多个节点， 每个节点并发
def mul_open_tx(ssiglist, permlinklist, userID):
    threads = []
    for i in range(n):
        # t = MyThread(open_tx,
        #              args=(accountlist[i], ssiglist[i], userID, permlinklist[i], clientlist[i].steemd, clientlist[i].wallet))
        t = MyThread(one_mul_open_tx,
                     args=(accountlist[i], ssiglist[i], userID, permlinklist[i], clientlist[i].steemd, clientlist[i].wallet))
        threads.append(t)
    for t in threads:
        t.start()
    for t in threads:
        t.join()
    # for t in threads:
    #     t.get_result()



#仅创造tx 不广播
def creat_commit_tx(account, usk, pk, GID, UID, steemd_instance, wallet_instance, title="paper_title",
                    body="paper_body"):
    commitop, ssig, permlink = annoy_commit(account, usk, pk, GID, UID, title="paper_title", body="paper_body",
                                            groupID="computer")
    commit_tx = tx_build(commitop, steemd_instance, wallet_instance, account)
    return ssig, permlink, commit_tx

def creat_num_commit_tx(num, account, usk, pk, GID, UID, steemd_instance, wallet_instance, title="paper_title",
                    body="paper_body"):
    ssiglist = []
    permlinklist = []
    txlist=[]
    for i in range(num):
        ssig, permlink, commit_tx = creat_commit_tx(account, usk, pk, GID, UID, steemd_instance, wallet_instance, title="paper_title",
                    body="paper_body")
        ssiglist.append(ssig)
        permlinklist.append(permlink)
        txlist.append(commit_tx)
    return ssiglist, permlinklist, txlist


def creat_open_tx(account, ssig, userID, permlink, steemd_instance, wallet_instance):
    openop = open_op(account, ssig, userID, permlink)
    open_tx = tx_build(openop, steemd_instance, wallet_instance, account)
    return open_tx

def tx_broad(tx):
    tx.broadcast()

def mul_tx_broad(txlist):
    threads = []
    for tx in txlist:
        t = MyThread(tx_broad, args=(tx,))
        threads.append(t)
    for t in threads:
        t.start()
    for t in threads:
        t.join()


# public parma
nodeTX = 5
k = 2
n = 2 #(k,n)
#节点地址
nodelist =[
    'http://101.76.218.202:8090',
    'http://101.76.218.202:8094'
]
accountlist = ["initminer2" , "zy1",  "zy2" ,  "zy3" ,  "zy4", "zy5", "zy6", "zy7", "zy8", "zy9", "zy10", "zy11",  "zy12" ,  "zy13" ,  "zy14", "zy15", "zy16", "zy17", "zy18", "zy19", "zy20"]
# 除了第一个 其他的都是posting key      5Hs4jcm5X4sanCnUKNFCjrq2irN8sH1Krzsb13Qd6DHqutZbhqu
keylist=['5J3yMruND2TADZ7cZc6Cnp4VePrnehei2wvGdnLgf3aEj2nDGhc', '5Hs4jcm5X4sanCnUKNFCjrq2irN8sH1Krzsb13Qd6DHqutZbhqu']
debug = True
#群签名相关
groupobj = PairingGroup('SS512')
group_signature = GroupSignature(groupobj)
L = group_signature.LGen(n, k)
#密钥相关
clientlist = []
for i in range(n):
    clientlist.append(steem.Steem(nodes=[nodelist[i]], keys=keylist[i]))

vkliststr= []
uskliststr = []
vklist = []
usklist = []
#steem testchain信息
steembase.chains.known_chains['TEST'] = {
        'chain_id': '18dcf0a285365fc58b71f18b3d3fec954aa0c141c44e4e5cb4cf777b9eab274e',
        'prefix': 'TST', 'steem_symbol': 'TESTS', 'sbd_symbol': 'TBD', 'vests_symbol': 'VESTS'
 }
groupID = "computer"
GID = group_signature.group.hash(groupID)



def main():
    # 假设不存在不可用节点(无法判断节点状态)
    userID = "zhou"
    UID = group_signature.group.hash(userID)
    print("uid", UID)
    #获取usk
    pk , usk = get_usk(userID, GID, UID)
    ##################################################################
    commitop, ssig, permlink = annoy_commit(accountlist[1], usk, pk, GID, UID,title = "paper_title",  body = "paper_body", groupID="computer")
    tx = tx_build(commitop, clientlist[1].steemd, clientlist[1].wallet, accountlist[1])
    tx.broadcast()
    re = tx_build_broad(commitop, clientlist[1].steemd, clientlist[1].wallet, accountlist[1])
    print("commit-re", re)

    #统一到一个函数  --1
    # ssig, permlink = annoy_commit_tx(accountlist[1], usk, pk, GID, UID, clientlist[1].steemd, clientlist[1].wallet, title="paper_title",
    #                 body="paper_body")
    # --1

    #多线程 多节点   --2
    # ssiglist,permlinklist = mul_annoy_tx(usk, pk, UID)
    # print(permlinklist)
    #--2

    #先创建交易，再统一广播 --3
    # _,_,txlist =creat_num_commit_tx(50, accountlist[1], usk, pk, GID, UID, clientlist[1].steemd, clientlist[1].wallet, title="paper_title",
    #                 body="paper_body"
    # )
    #
    # for tx in txlist:
    #     tx.broadcast()

    # _, _, txlist2 = creat_num_commit_tx(50, accountlist[0], usk, pk, GID, UID, clientlist[0].steemd,
    #                                    clientlist[0].wallet, title="paper_title",
    #                                    body="paper_body"
    #                                    )
    # print(txlist)
    # txlist.extend(txlist2)
    # mul_tx_broad(txlist)
    # --3


    sleep(3)
    #多线程 单节点
    # one_mul_annoy_tx(accountlist[1], usk, pk, UID, clientlist[1].steemd, clientlist[1].wallet)
    ######################################################################################

    # openop = open_op(accountlist[0], ssig, userID, permlink)
    # re = tx_build_broad(openop, clientlist[0].steemd, clientlist[0].wallet, accountlist[0])
    # print("open-re", re)

    # 统一到一个函数
    # open_tx(accountlist[1], ssig, userID, permlink, clientlist[1].steemd, clientlist[1].wallet)

    # 多线程 多节点
    # mul_open_tx(ssiglist, permlinklist, userID)
    ###########################################################################################3
    return

if __name__ == "__main__":
    main()


print("end")



# clientlist[0].get_block(992)
# clientlist[0].get_account("zy1")
# clientlist[0].get_account_count()
# clientlist[0].get_account_history("zy1", index_from=-1, limit=3)
# clientlist[0].get_active_witnesses()
# clientlist[0].get_blocks_range(292,299)

# cli = steem.Steem(nodes=[nodelist[0]], keys="5J3yMruND2TADZ7cZc6Cnp4VePrnehei2wvGdnLgf3aEj2nDGhc")


# cli.create_account('zy2', "","test",creator='zy1')
# cli.create_account('zy2', "","test", owner_key="" ,
#                              active_key="" , posting_key="" ,
#                              memo_key="",creator='zy1')

# cli = steem.Steem(nodes=[nodelist[1]], keys=["5K9tB6y5zGhsjAbX7VzEHz2i5gdjLvFz7AZYNmJqTXJT2eccsps"])
#
# cli.create_account('zy2',delegation_fee_steem= "1 STEEM",password="test",creator='zy1')

# cli = steem.Steem(nodes=[nodelist[0]], keys=["5J3yMruND2TADZ7cZc6Cnp4VePrnehei2wvGdnLgf3aEj2nDGhc"])
#
# cli.create_account('zy2',delegation_fee_steem= "2 STEEM",password="test",creator='initminer2')



































