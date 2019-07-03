
#include <steem/plugins/group_signature/group_signature.hpp>
#include <steem/plugins/group_signature/group_signature_plugin.hpp>
#include <fc/log/logger.hpp>

namespace steem
{
namespace plugins
{
namespace group_signature
{

using namespace appbase;

namespace detail
{

group_signature_plugin_impl::group_signature_plugin_impl()
{
}

group_signature_plugin_impl::~group_signature_plugin_impl()
{
    CLEAR_ELEMENT(Ei, K);
}

void group_signature_plugin_impl::L(element_t I, element_t *l)
{
    ZR_ELEMENT(temp, pairing);
    element_set1(temp);

    ZR_ELEMENT(J, pairing);
    element_set1(J);

    ZR_ELEMENT(temp1, pairing);
    ZR_ELEMENT(temp2, pairing);
    for (int i = 0; i < N; i++)
    {
        if (element_cmp(J, I))
        {
            element_sub(temp1, J, I);
            element_div(temp2, J, temp1);
            element_mul(*l, *l, temp2);
        }
        element_add(J, J, temp);
    }
    element_clear(temp);
    element_clear(J);
    element_clear(temp1);
    element_clear(temp2);
}

void group_signature_plugin_impl::GenSiTi(element_t *F, element_t *temp)
{
    ZR_ELEMENT(sum, pairing);
    ZR_ELEMENT(temp1, pairing);

    for (int i = 0; i < N; i++)
    {
        element_set0(sum);
        int pre = 1;
        for (int j = 0; j < K; j++)
        {
            element_mul_si(temp1, F[j], pre);
            element_add(sum, sum, temp1);
            pre *= (i + 1);
        }
        element_init_Zr(temp[i], pairing);
        element_set(temp[i], sum);
    }

    element_clear(sum);
    element_clear(temp1);
}
void group_signature_plugin_impl::EiGen(element_t *F, element_t *G, int threshold, element_t *temp)
{
    G1_ELEMENT(temp1, pairing);
    G1_ELEMENT(temp2, pairing);

    for (int i = 0; i < threshold; i++)
    {
        element_pow_zn(temp1, mpk->g, F[i]);
        element_pow_zn(temp2, mpk->h, G[i]);
        element_mul(temp[i], temp1, temp2);
    }
    element_clear(temp1);
    element_clear(temp2);
}
void group_signature_plugin_impl::SharesGen(element_t *s_ptr, element_t *t_ptr, element_t *E_ptr)
{
    ZR_ELEMENT(s, pairing);
    ZR_ELEMENT(t, pairing);

    element_random(s);
    element_random(t);
    //测试，先固定s，t
    // const char sstr[] = "346122343231638985077046426490987973974719772237";
    // element_set_str(s, sstr, 10);
    // const char tstr[] = "77583513436316961525400618262687645185192876454";
    // element_set_str(t, tstr, 10);

    ZR_ELEMENT_ARRAY(F, K, pairing);
    ZR_ELEMENT_ARRAY(G, K, pairing);

    FGGen(s, K, F); //多项式的系数
    FGGen(t, K, G);

    EiGen(F, G, K, E_ptr);

    GenSiTi(F, s_ptr);
    GenSiTi(G, t_ptr);

    element_clear(s);
    element_clear(t);
    for (int i = 0; i < K; i++)
    {
        element_clear(F[i]);
        element_clear(G[i]);
    }
}
void group_signature_plugin_impl::FGGen(element_t s, int threshold, element_t *temp)
{
    element_set(temp[0], s);
    for (int i = 1; i < threshold; i++)
    {
        element_random(temp[i]);
    }
}

void group_signature_plugin_impl::GrpSetup(element_t alpha, element_t r1)
{

    element_pow_zn(gsk->a2, mpk->u2, r1);
    element_pow_zn(gsk->a3, mpk->u3, r1);
    element_pow_zn(gsk->a4, mpk->u4, r1);
    element_pow_zn(gsk->a5, mpk->g, r1);

    G2_ELEMENT(temp, pairing);
    G2_ELEMENT(temp1, pairing);
    element_pow_zn(temp1, mpk->g2, alpha);
    element_pow_zn(temp, mpk->u1, GID);
    element_mul(temp, mpk->u0, temp);
    element_pow_zn(temp, temp, r1);
    element_mul(gsk->a0, temp1, temp);

    element_pow_zn(vk, mpk->g, alpha);
    char temp_str[400];
    element_snprintf(temp_str, 320, "%B",vk);
    vk_string = temp_str;
    element_printf("%B\n",vk);
    g_alpha_i_string_received[number] = vk_string;
    std::cout<<"this is vk "<<vk_string<<"\n\n";
    element_clear(temp);
    element_clear(temp1);
}
bool group_signature_plugin_impl::VerifyShare(std::string Si_string, std::string Ti_string, std::vector<std::string> Ei_string)
{
    element_t Si,Ti;
    element_init_Zr(Si, pairing);
    element_init_Zr(Ti, pairing);
    G1_ELEMENT_ARRAY(Ei, N, pairing);

    element_set_str(Si, Si_string.c_str(), 10);
    element_set_str(Ti, Ti_string.c_str(), 10);
    // element_printf("%B\n",Si);
    // element_printf("%B\n",Ti);

    // for(int i = 0; i< K; i++){
    //     element_set_str(Ei[i], Ei_string[i].c_str(), 10);
    //     element_printf("%B\n",Ei[i]);
    // }

    G1_ELEMENT(result1, pairing);
    G1_ELEMENT(result2, pairing);
    G1_ELEMENT(result11, pairing);
    G1_ELEMENT(result12, pairing);

    element_pow_zn(result11, mpk->g, Si);
    element_pow_zn(result12, mpk->h, Ti);
    element_mul(result1, result11, result12);

    element_set1(result2);

    ZR_ELEMENT(I, pairing);
    ZR_ELEMENT(pre, pairing);
    element_set1(pre);
    element_set_si(I, number+1);

    G1_ELEMENT(temp, pairing);
    for (int j = 0; j < K; j++)
    {
        element_pow_zn(temp, Ei[j], pre);
        element_mul(result2, result2, temp);
        element_mul(pre, pre, I);
    }
    // element_printf("re1 is %B\n",result1);
    // element_printf("re2 is %B\n",result2);

    bool re = element_cmp(result1, result2) == 0 ? true : false;
    CLEAR_ELEMENT(Ei,N);

    element_clear(Si);
    element_clear(Ti);
    element_clear(result1);
    element_clear(result2);
    element_clear(result11);
    element_clear(result12);
    element_clear(I);
    element_clear(pre);
    element_clear(temp);
    return re;
}
//生成某个用户的私钥
void group_signature_plugin_impl::Extract_administrator(element_t UID, usktype *usk)
{
    G2_ELEMENT(temp1, pairing);
    element_pow_zn(temp1, gsk->a2, UID);
    element_mul(usk->b0, gsk->a0, temp1);
    element_set(usk->b3, gsk->a3);
    element_set(usk->b4, gsk->a4);
    element_set(usk->b5, gsk->a5);

    element_clear(temp1);
}
void group_signature_plugin_impl::openkey(const char *e1, const char *e2, oktype *ok)
{
    G1_ELEMENT(e1e, pairing);
    element_set_str(e1e, e1, 10);
    G2_ELEMENT(e2e, pairing);
    element_set_str(e2e, e2, 10);
    pairing_apply(ok->ok1, e1e, gsk->a0, pairing);
    pairing_apply(ok->ok2, gsk->a5, e2e, pairing);
}
//验证
bool group_signature_plugin_impl::Verify(element_t m, signaturetype *sig)
{
    element_t d1, d2, q, t, M, temp1, temp2;
    element_init_G1(d1, pairing);
    element_init_G2(d2, pairing);
    element_init_GT(q, pairing);
    element_init_Zr(t, pairing);
    element_random(t);
    element_init_GT(M, pairing);
    element_random(M);
    element_pow_zn(d1, mpk->g, t);
    element_init_G2(temp1, pairing);
    element_init_G2(temp2, pairing);
    element_pow_zn(temp1, mpk->u1, GID);
    element_pow_zn(temp2, mpk->u3, m);
    element_mul(temp1, mpk->u0, temp1);
    element_mul(temp1, temp1, temp2);
    element_mul(temp1, temp1, sig->c6);
    element_pow_zn(d2, temp1, t);

    element_clear(temp1);
    element_init_GT(temp1, pairing);
    pairing_apply(temp1, mpk->h1, mpk->g2, pairing);
    element_pow_zn(temp1, temp1, t);
    element_mul(q, M, temp1);

    element_clear(temp1);
    element_clear(temp2);
    element_init_GT(temp1, pairing);

    element_init_GT(temp2, pairing);
    pairing_apply(temp1, sig->c5, d2, pairing);

    pairing_apply(temp2, d1, sig->c0, pairing);
    element_div(temp1, temp1, temp2);
    element_mul(temp1, q, temp1);
    return element_cmp(M, temp1) == 0 && VerifyPOK(sig) ? true : false;
}
bool group_signature_plugin_impl::VerifyPOK(signaturetype *sig)
{
    // printf("this is verifypok\n");
    element_t temp1, temp2, temp3, negc, r1Pr, r2Pr, r3Pr, r4Pr, f, g3;
    element_init_G2(temp1, pairing);
    element_init_G2(temp2, pairing);
    element_init_G2(temp3, pairing);
    element_init_Zr(negc, pairing);
    element_init_G2(f, pairing);
    element_init_GT(g3, pairing);
    element_init_G2(r1Pr, pairing);
    element_init_G2(r2Pr, pairing);
    element_init_G2(r3Pr, pairing);
    element_init_GT(r4Pr, pairing);

    element_pow_zn(temp1, mpk->u2, sig->p->s1);
    element_pow_zn(temp2, mpk->u4, sig->p->s2);
    element_neg(negc, sig->p->c);
    element_pow_zn(temp3, sig->c6, negc);

    element_mul(temp1, temp1, temp2);
    element_mul(r1Pr, temp1, temp3);

    element_clear(temp1);
    element_clear(temp2);
    element_init_G1(temp1, pairing);
    element_init_G1(temp2, pairing);
    element_pow_zn(temp1, sig->e1, negc);
    element_pow_zn(temp2, mpk->g, sig->p->s3);
    element_mul(r2Pr, temp2, temp1);

    element_clear(temp1);
    element_init_G2(temp1, pairing);
    element_pow_zn(temp1, mpk->u1, GID);
    element_mul(f, mpk->u0, temp1);
    element_pow_zn(temp1, f, sig->p->s3);
    element_init_G2(temp2, pairing);
    element_pow_zn(temp2, sig->e2, negc);
    element_mul(r3Pr, temp1, temp2);

    element_clear(temp1);
    element_clear(temp2);
    element_clear(temp3);
    element_init_GT(temp1, pairing);
    element_init_GT(temp2, pairing);
    element_init_GT(temp3, pairing);

    element_pow_zn(temp1, mpk->n, sig->p->s1);
    pairing_apply(g3, mpk->h1, mpk->g2, pairing);
    element_pow_zn(temp2, g3, sig->p->s3);
    element_pow_zn(temp3, sig->e3, negc);
    element_mul(temp1, temp1, temp2);
    element_mul(r4Pr, temp1, temp3);

    char a[2500] = {'\0'};
    char b[520];
    element_snprintf(b, 500, "%B", r1Pr);
    // printf("%zd\n",strlen(b));
    strcat(a, b);
    element_snprintf(b, 500, "%B", r2Pr);
    // printf("%zd\n",strlen(b));

    strcat(a, b);
    element_snprintf(b, 500, "%B", r3Pr);
    // printf("%zd\n",strlen(b));

    strcat(a, b);
    element_snprintf(b, 500, "%B", r4Pr);
    // printf("%zd\n",strlen(b));

    strcat(a, b);

    element_clear(temp1);
    element_init_Zr(temp1, pairing);
    element_from_hash(temp1, a, 1500);
    bool re = element_cmp(temp1, sig->p->c) == 0 ? true : false;
    element_clear(temp1);
    element_clear(temp2);
    element_clear(temp3);
    element_clear(negc);
    element_clear(r1Pr);
    element_clear(r2Pr);
    element_clear(r3Pr);
    element_clear(r4Pr);
    element_clear(f);
    element_clear(g3);
    return re;
}
bool group_signature_plugin_impl::open(char *userID, const char *lameda, element_t e)
{
    ZR_ELEMENT(UID, pairing);
    element_from_hash(UID, (void *)userID, (int)strlen(userID));
    GT_ELEMENT(temp, pairing);
    element_pow_zn(temp, mpk->n, UID);
    GT_ELEMENT(la, pairing);
    element_set_str(la, lameda, 10);
    element_mul(temp, temp, la);
    bool re = element_cmp(e, temp) == 0 ? true : false;
    element_clear(UID);
    element_clear(temp);
    element_clear(la);
    return re;
}
void group_signature_plugin_impl::mpkGen()
{
    for(int i = 0; i< N;i++){
        if(si_string_received[i] == "")
        {
            ilog("si is not n");
            return;
        }
    }
    ZR_ELEMENT(alpha_i, pairing);
    ZR_ELEMENT(ri, pairing);
    element_set0(alpha_i);
    element_set0(ri);

    ZR_ELEMENT_ARRAY(ss, N, pairing);
    ZR_ELEMENT_ARRAY(tt, N, pairing);

    for (int i = 0; i < N; i++)
    {
        element_set_str(ss[i], (const char *)si_string_received[i].c_str(), 10);
        element_set_str(tt[i], (const char *)ti_string_received[i].c_str(), 10);
    }

    for (int i = 0; i < N; i++)
    {
        element_add(alpha_i, alpha_i, ss[i]);
        element_add(ri, ri, tt[i]);
    }
    element_printf("ai %B\n",alpha_i);
    element_printf("ri is %B\n",ri);
    GrpSetup(alpha_i, ri);
    CLEAR_ELEMENT(ss, N);
    CLEAR_ELEMENT(tt, N);
    element_clear(alpha_i);
    element_clear(ri);
}
void group_signature_plugin_impl::g_alphaGen()
{
    std::cout<<"this is g_alpha\n\n";
    G1_ELEMENT_ARRAY(g_temp,N,pairing);
    int j = 0;
    for(int i = 0; i< N; i++){
        if(g_alpha_i_string_received[i] != "")
        {
            element_set_str(g_temp[i],(const char*)g_alpha_i_string_received[i].c_str(),10);
            j++;
        }
        else
            element_set1(g_temp[i]);
    }

    if(j < K)
    {
        std::cout<<"vk is not K j is "<<j<<"\n\n";
        return;
    }

    ZR_ELEMENT(I, pairing);
    ZR_ELEMENT(e, pairing);

    element_set1(I);
    element_set1(e);

    ZR_ELEMENT_ARRAY(Li, N, pairing);
    INIT_ARRAY_1(Li, N);
    for (int i = 0; i < N; i++)
    {
        element_set1(Li[i]);
        L(I, &Li[i]);
        element_add(I, I, e);
    }
    element_clear(I);
    element_clear(e);

    G1_ELEMENT(g_alpha, pairing);
    element_set1(g_alpha);

    G1_ELEMENT(temp, pairing);
    for (int i = 0; i < N; i++)
    {
        element_pow_zn(temp, g_temp[i], Li[i]);
        element_mul(g_alpha, g_alpha, temp);
    }
    element_set(mpk->h1, g_alpha);
    element_printf("h1 is %B\n",mpk->h1);
    element_clear(g_alpha);
    CLEAR_ELEMENT(Li,N);
    CLEAR_ELEMENT(g_temp,N);
}
bool group_signature_plugin_impl::VerifyShareSelf(element_t Si, element_t Ti, element_t *Ei, int threshold, int i)
{
    std::cout<<"verify self\n\n";
    element_printf("%B\n",Si);
    element_printf("%B\n",Ti);
    for(int i = 0; i< K; i++){
        element_printf("%B\n",Ei[i]);
    }
    element_t result1, result2, result11, result12;
    element_init_G1(result1, pairing);
    element_init_G1(result2, pairing);
    element_init_G1(result11,pairing);
    element_init_G1(result12, pairing);

    element_pow_zn(result11, mpk->g, Si);
    element_pow_zn(result12, mpk->h, Ti);
    element_mul(result1, result11, result12);

    element_set1(result2);

    element_t I, pre;
    element_init_Zr(I, pairing);
    element_init_Zr(pre, pairing);
    element_set1(pre);
    element_set_si(I, i);

    element_t temp;
    element_init_G1(temp, pairing);
    for (int j = 0; j < threshold; j++)
    {
        element_pow_zn(temp, Ei[j], pre);
        element_mul(result2, result2, temp);
        element_mul(pre, pre, I);
    }

    bool ok= element_cmp(result1, result2) == 0 ? true : false;

    element_clear(result1);
    element_clear(result2);
    element_clear(result11);
    element_clear(result12);
    element_clear(I);
    element_clear(pre);
    element_clear(temp);
    return ok;
}
} // namespace detail

group_signature_plugin::group_signature_plugin() {}
group_signature_plugin::~group_signature_plugin() {}
void group_signature_plugin::set_program_options(bpo::options_description &cli, bpo::options_description &cfg)
{

    cfg.add_options()
        ("k-number", bpo::value<int>()->default_value(21), "the value of threshold")
        ("n-number", bpo::value<int>()->default_value(21), "the number of node in this network")
        ("node-number", bpo::value<int>()->default_value(0), "the number of this node");
}
void group_signature_plugin::plugin_initialize(const boost::program_options::variables_map &options)
{ //初始化公钥
    ilog("group_signature initialize start");
    my = std::make_unique<detail::group_signature_plugin_impl>();

    if ( options.count( "k-number" ) )
    {
        my->K = options.at( "k-number" ).as< int >();

    }
    if ( options.count( "n-number" ) )
    {
        my->N = options.at( "n-number" ).as< int >();

    }
    if (options.count( "node-number" ) )
    {
        my->number = options.at( "node-number" ).as< int >();

    }

    char params[] = {"type a\n\
q 8780710799663312522437781984754049815806883199414208211028653399266475630880222957078625179422662221423155858769582317459277713367317481324925129998224791\n\
h 12016012264891146079388821366740534204802954401251311822919615131047207289359704531102844802183906537786776\n\
r 730750818665451621361119245571504901405976559617\n\
exp2 159\n\
exp1 107\n\
sign1 1\n\
sign0 1\n"};
    pairing_init_set_str(my->pairing, (const char *)params);
    //管理员现在获得的ai,ri
    my->mpk = new mpktype(my->pairing);
    const char gstr[] = "[6172776968119684165170291368128433652817636448173749093457023424948260385279837018774774149930982188956916913145008943931711059687988096415181819433817738, 8687587692191287108886119971783525001480020593934954052605681527814232399216375005546606067382536684351686344089456732201641997200939472924879001214689004]";
    const char g2str[] = "[7648994551207171188393784904797547917038803147671542540175090956205316897431443264058433935237605598252399113847934759009659621851760599508222321653067284, 922489308494109901795721463782161260386164061515796674638135394871842997698175772871045949554746517321480649326465484116060959631197509151923296896589720]";
    const char u0str[] = "[180015966842918451436547451263180245588308971597733548673037049536176684754209695288737508087729924028686259002375511049961436438196866049956546630518033, 1295050197915669955783867959538729894307963685491173858450359845766785488725907727220684060845012524740394664162328817669422178637925195059862486690053923]";
    const char u1str[] = "[2555472719769037960206282327195096320915753855199743796256065902544200822503613205017219993060986152240852358189992579821797745072366030183800897743028220, 7573705235093543416041007636313631591000596820214067724084077929638801811700093589294454562385664531190678890366928407286293582994146887505184778221562373]";
    const char u2str[] = "[6876276970903121931083294698771200898345396507892092532649392211995185517437159402176975528760594250374462299539306423347676182899798006533425047523984724, 5323739238507219125881988073888745575030677585404965990610324901624530474522642705344792075909082041735695801098770187248023797265998906693745587936574078]";
    const char u3str[] = "[6628726193389375981104409894060310698729022957801238449570622103067828518416602275957863668289683360250722835022304456841105526036470008237775051984811323, 862537748555943361001122447731987661405436458862545177179548603003392540530328380518694788420155531238391922289886044667763424887444361610972254938158280]";
    const char u4str[] = "[8157254219580822599577995921928211211847392705248772673869189421041858895589817404931780741226510985762564598862965174380020566416411083236239871342674775, 4736677719200783513058679582227494204159737596114643136852532046080608159561620208171676599501713934575216178076006396924589443776642926902969084668055006]";
    const char hstr[] = "[6248393417805371388321299785844751688345516419281230263497475615452026459314582553252281068616984105757749673095320346188725995701858182333525688832492249, 351368339412205819108519989143352052898751906937356995136442397753142226531384069336237369861919799955237545207977716196031001184146017796598836939617335]";
    const char nstr[] = "[75201312764006187596691102237923705656296213254701583615255122742135170369075831428394751330697143847448434841509551532135632624530360013837581615049543, 3886258599652934715331576083899336629981754505948456216299528998628273512432828729344158706718479567056972375128622026273382126529171409058157562418608963]";
    element_set_str(my->mpk->g, gstr, 10);
    element_set_str(my->mpk->h, hstr, 10);
    element_set_str(my->mpk->g2, g2str, 10);
    element_set_str(my->mpk->u0, u0str, 10);
    element_set_str(my->mpk->u1, u1str, 10);
    element_set_str(my->mpk->u2, u2str, 10);
    element_set_str(my->mpk->u3, u3str, 10);
    element_set_str(my->mpk->u4, u4str, 10);
    element_set_str(my->mpk->n, nstr, 10);

    element_init_Zr(my->GID, my->pairing);
    std::string grpID = "computer";
    element_from_hash(my->GID, (void *)grpID.c_str(), 8);
    my->gsk = new gsktype(my->pairing);
    element_init_G1(my->vk, my->pairing);
    //产生要发送的秘钥分享，放在p2p中，当P2P节点启动后，每当有新的节点建立链接，则将秘钥发送出去。
    my->Ei = new element_t[my->K];
    for (int i = 0; i < my->K; i++)
    {
        element_init_G1(my->Ei[i], my->pairing);
    }

    G1_ELEMENT_ARRAY(s, my->N, my->pairing);
    G1_ELEMENT_ARRAY(t, my->N, my->pairing);
    my->SharesGen(s, t, my->Ei); //本节点的share
    // for(int i = 0; i< my->N; i++){
    //     std::cout<<"this is verify share\n\n";
    //     if(!my->VerifyShareSelf(s[i], t[i], my->Ei,my->K,i+1))
    //     {
    //         std::cout<<"share is invalid\n\n";
    //     }
    // }
    my->g_alpha_i_string_received.assign(my->N,"");

    my->si_string_received.assign(my->N,"");

    my->ti_string_received.assign(my->N,"");

    //转化为字符串
    char temp[400];
    element_snprintf(temp, 320,"%B", s[my->number]);

    my->si_string_received[my->number] = temp;

    element_snprintf(temp, 320,"%B", t[my->number]);

    my->ti_string_received[my->number] = temp;
    for (int i = 0; i < my->N; i++)
    {
        element_snprintf(temp, 320, "%B", s[i]);
        my->si_string.push_back(temp);
        element_snprintf(temp, 320, "%B", t[i]);
        my->ti_string.push_back(temp);
    }
    for (int i = 0; i < my->K; i++)
    {
        element_snprintf(temp, 320, "%B", my->Ei[i]);
        my->Ei_string.push_back(temp);
    }

    CLEAR_ELEMENT(s, my->N);
    CLEAR_ELEMENT(t, my->N);

    ilog("group signature initialize endup");
}

void group_signature_plugin::plugin_startup()
{
    ilog("group_signature: plugin_startup() begin");
    ilog("group_signature:plugin_start():end");
}

void group_signature_plugin::plugin_shutdown()
{
    if (my->gsk != NULL)
    {
        delete my->gsk;
    }
    element_clear(my->vk);
    element_clear(my->GID);
    delete my->mpk;
}

} // namespace group_signature
} // namespace plugins
} // namespace steem