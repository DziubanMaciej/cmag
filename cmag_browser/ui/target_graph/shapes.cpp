#include "shapes.h"

const static float staticLibVertices[] = {

    1.0,
    -0.16577777777777733,
    1.0,
    0.16555555555555534,
    0.4142156862745099,
    0.4,
    -0.41421568627451,
    0.4,
    -1.0,
    0.16555555555555534,
    -1.0,
    -0.16577777777777733,
    -0.41421568627451,
    -0.4,
    0.4142156862745099,
    -0.4,
};
const ShapeInfo ShapeInfo::staticLib(staticLibVertices);

const static float executableVertices[] = {
    0.09259589652096345,
    -0.3988888888888887,
    0.15379125780553093,
    -0.39688888888888924,
    0.21445138269402286,
    -0.3935555555555564,
    0.27421944692239064,
    -0.3891111111111109,
    0.332738626226583,
    -0.38377777777777866,
    0.3900089206066013,
    -0.37733333333333374,
    0.44585191793041945,
    -0.36977777777777876,
    0.49973238180196233,
    -0.3613333333333332,
    0.5514719000892063,
    -0.3517777777777787,
    0.6012488849241746,
    -0.34133333333333366,
    0.6485280999107943,
    -0.3297777777777785,
    0.6933095450490634,
    -0.31755555555555603,
    0.7354148082069585,
    -0.3044444444444442,
    0.7746654772524533,
    -0.29022222222222227,
    0.8108831400535232,
    -0.27555555555555633,
    0.8440677966101693,
    -0.2599999999999998,
    0.8742194469223905,
    -0.2437777777777784,
    0.9013380909901869,
    -0.22666666666666643,
    0.9248884924174838,
    -0.2091111111111117,
    0.9454058876003568,
    -0.19111111111111165,
    0.9625334522747542,
    -0.17244444444444423,
    0.9764495985727031,
    -0.15311111111111209,
    0.9871543264942011,
    -0.13355555555555537,
    0.9946476360392504,
    -0.11377777777777792,
    0.9987511150758253,
    -0.09355555555555639,
    1.0,
    -0.07311111111111157,
    0.998037466547725,
    -0.052222222222222725,
    0.993220338983051,
    -0.03155555555555589,
    0.9855486173059769,
    -0.010444444444445034,
    0.9752007136485283,
    0.010444444444443757,
    0.9619982158786797,
    0.03133333333333266,
    0.9464763603925066,
    0.052222222222221504,
    0.9284567350579842,
    0.07288888888888828,
    0.9082961641391618,
    0.0933333333333331,
    0.8859946476360394,
    0.11355555555555452,
    0.8617305976806424,
    0.13355555555555543,
    0.8355040142729702,
    0.15311111111111075,
    0.8076717216770735,
    0.17222222222222228,
    0.7782337198929525,
    0.19088888888888833,
    0.7473684210526312,
    0.20888888888888846,
    0.7150758251561102,
    0.22666666666666646,
    0.6817127564674392,
    0.24355555555555508,
    0.6472792149866184,
    0.2597777777777778,
    0.6117752007136488,
    0.2753333333333331,
    0.5755575379125777,
    0.29022222222222227,
    0.5384478144513827,
    0.30422222222222217,
    0.5008028545941121,
    0.3173333333333328,
    0.46244424620874236,
    0.32977777777777717,
    0.42355040142729705,
    0.34111111111111037,
    0.38429973238180226,
    0.3515555555555555,
    0.34469223907225666,
    0.36111111111111105,
    0.30472792149866157,
    0.3695555555555554,
    0.2645851917930415,
    0.3771111111111105,
    0.22426404995539673,
    0.3835555555555554,
    0.1837644959857272,
    0.38911111111111096,
    0.1430865298840318,
    0.3933333333333331,
    0.10223015165031257,
    0.39666666666666595,
    0.061373773416592226,
    0.3988888888888886,
    0.0205173951828721,
    0.4,
    -0.02051739518287199,
    0.4,
    -0.061373773416592226,
    0.3988888888888886,
    -0.10223015165031257,
    0.39666666666666595,
    -0.1430865298840318,
    0.3933333333333331,
    -0.1837644959857272,
    0.38911111111111096,
    -0.22426404995539684,
    0.3835555555555554,
    -0.26458519179304163,
    0.3771111111111105,
    -0.30472792149866157,
    0.3695555555555554,
    -0.34469223907225677,
    0.36111111111111105,
    -0.38429973238180226,
    0.3515555555555555,
    -0.42355040142729716,
    0.34111111111111037,
    -0.46244424620874236,
    0.32977777777777717,
    -0.5008028545941121,
    0.3173333333333328,
    -0.5384478144513827,
    0.30422222222222217,
    -0.5755575379125778,
    0.29022222222222227,
    -0.6117752007136488,
    0.2753333333333331,
    -0.6472792149866184,
    0.2597777777777778,
    -0.6817127564674392,
    0.24355555555555508,
    -0.7150758251561102,
    0.22666666666666646,
    -0.7473684210526312,
    0.20888888888888846,
    -0.7782337198929526,
    0.19088888888888833,
    -0.8076717216770736,
    0.17222222222222228,
    -0.8355040142729702,
    0.15311111111111075,
    -0.8617305976806423,
    0.13355555555555543,
    -0.8859946476360395,
    0.11355555555555452,
    -0.9082961641391617,
    0.0933333333333331,
    -0.9284567350579841,
    0.07288888888888828,
    -0.9464763603925067,
    0.052222222222221504,
    -0.9619982158786798,
    0.03133333333333266,
    -0.9752007136485283,
    0.010444444444443757,
    -0.9855486173059768,
    -0.010444444444445034,
    -0.993220338983051,
    -0.03155555555555589,
    -0.998037466547725,
    -0.052222222222222725,
    -1.0,
    -0.07311111111111157,
    -0.9987511150758253,
    -0.09355555555555639,
    -0.9946476360392504,
    -0.11377777777777792,
    -0.9871543264942011,
    -0.13355555555555537,
    -0.9764495985727031,
    -0.15311111111111209,
    -0.9625334522747543,
    -0.17244444444444423,
    -0.9454058876003568,
    -0.19111111111111165,
    -0.9248884924174837,
    -0.2091111111111117,
    -0.9013380909901868,
    -0.22666666666666643,
    -0.8742194469223905,
    -0.2437777777777784,
    -0.8440677966101694,
    -0.2599999999999998,
    -0.8108831400535232,
    -0.27555555555555633,
    -0.7746654772524533,
    -0.29022222222222227,
    -0.7354148082069585,
    -0.3044444444444442,
    -0.6933095450490634,
    -0.31755555555555603,
    -0.6485280999107942,
    -0.3297777777777785,
    -0.6012488849241745,
    -0.34133333333333366,
    -0.5514719000892063,
    -0.3517777777777787,
    -0.4997323818019622,
    -0.3613333333333332,
    -0.44585191793041934,
    -0.36977777777777876,
    -0.3900089206066014,
    -0.37733333333333374,
    -0.3327386262265829,
    -0.38377777777777866,
    -0.27421944692239075,
    -0.3891111111111109,
    -0.21445138269402286,
    -0.3935555555555564,
    -0.15379125780553082,
    -0.39688888888888924,
    -0.09259589652096334,
    -0.3988888888888887,
    -0.03086529884032141,
    -0.4,
    0.03086529884032152,
    -0.4,
};
const ShapeInfo ShapeInfo::executable(executableVertices);

const static float sharedLibVertices[] = {
    1.0,
    -0.19290909090909014,
    1.0,
    0.19290909090909114,
    0.3992914979757083,
    0.4,
    -0.3992914979757075,
    0.4,
    -1.0,
    0.19290909090909114,
    -1.0,
    -0.19290909090909014,
    -0.3992914979757075,
    -0.4,
    0.3992914979757083,
    -0.4,

    0.9493927125506072,
    -0.13563636363636328,
    0.9493927125506072,
    0.13545454545454527,
    0.39321862348178227,
    0.32727272727272727,
    -0.3932186234817816,
    0.32727272727272727,
    -0.9493927125506073,
    0.13545454545454527,
    -0.9493927125506073,
    -0.13563636363636328,
    -0.3932186234817816,
    -0.32727272727272727,
    0.39321862348178227,
    -0.32727272727272727,
};
const ShapeInfo ShapeInfo::sharedLib(sharedLibVertices, {0, 16});

const static float moduleLibVertices[] = {
    1.0,
    -0.2121538461538456,
    1.0,
    0.21200000000000052,
    0.38586891875817764,
    0.4,
    -0.38586891875817764,
    0.4,
    -1.0,
    0.21200000000000052,
    -1.0,
    -0.2121538461538456,
    -0.38586891875817764,
    -0.4,
    0.38586891875817764,
    -0.4,

    0.9524206018793862,
    -0.16338461538461546,
    0.9524206018793862,
    0.16338461538461546,
    0.38039728797430694,
    0.3384615384615385,
    -0.38039728797430716,
    0.3384615384615385,
    -0.9524206018793863,
    0.16338461538461546,
    -0.9524206018793863,
    -0.16338461538461546,
    -0.38039728797430716,
    -0.3384615384615385,
    0.38039728797430694,
    -0.3384615384615385,

    0.9048412037587725,
    -0.11476923076923046,
    0.9048412037587725,
    0.11461538461538445,
    0.374806708695135,
    0.27692307692307694,
    -0.374806708695135,
    0.27692307692307694,
    -0.9048412037587724,
    0.11461538461538445,
    -0.9048412037587724,
    -0.11476923076923046,
    -0.374806708695135,
    -0.27692307692307694,
    0.374806708695135,
    -0.27692307692307694,
};
const ShapeInfo ShapeInfo::moduleLib(moduleLibVertices, {0, 16, 24});

const static float customTargetVertices[] = {
    1.0,
    0.4,
    -1.0,
    0.4,
    -1.0,
    -0.4,
    1.0,
    -0.4,
};
const ShapeInfo ShapeInfo::customTarget(customTargetVertices);

const static float interfaceLibVertices[] = {
    1.0,
    0.4,
    -1.0,
    0.4,
    -1.0,
    -0.4,
    1.0,
    -0.4,
};
const ShapeInfo ShapeInfo::interfaceLib(interfaceLibVertices);

const static float objectLibVertices[] = {
    1.0,
    0.0,
    0.4999348534201955,
    0.4,
    -0.4999348534201955,
    0.4,
    -1.0,
    0.0,
    -0.4999348534201955,
    -0.4,
    0.4999348534201955,
    -0.4,
};
const ShapeInfo ShapeInfo::objectLib(objectLibVertices);

const static float unknownLibVertices[] = {
    1.0,
    -0.11455289304500343,
    0.8018618549592003,
    0.2414962010520164,
    2.220446049250313e-16,
    0.4,
    -0.8018618549592001,
    0.2414962010520164,
    -1.0,
    -0.11455289304500343,
    -0.4450063211125157,
    -0.4,
    0.4450063211125159,
    -0.4,
};
const ShapeInfo ShapeInfo::unknownLib(unknownLibVertices);
