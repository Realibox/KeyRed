def parse_playcanvs_to_gltf_animation(path):
    from datetime import datetime
    import json

    with open(path, "r") as fp:
        content = json.loads(fp.read())

    animation = content['animation']
    nodes = animation['nodes']

    u_nodes = []
    for node in nodes:
        keys = node['keys']
        px = []
        py = []
        pz = []
        sx = []
        sy = []
        sz = []
        qx = []
        qy = []
        qz = []
        qw = []

        for i in range(len(keys)):
            key = keys[i]
            p = [-key['p'][0], key['p'][1], key['p'][2]]
            r = key['r']
            s = key['s']
            u_q = playcanvs_euler_to_unity_quaternion(r)

            time = key['t']
            px.append((time, p[0]))
            py.append((time, p[1]))
            pz.append((time, p[2]))

            sx.append((time, s[0]))
            sy.append((time, s[1]))
            sz.append((time, s[2]))

            qx.append((time, u_q[0]))
            qy.append((time, u_q[1]))
            qz.append((time, u_q[2]))
            qw.append((time, u_q[3]))

        # 减帧处理
        import animation as ani_reduce
        error = 0.05
        new_px = ani_reduce.simplify(px, error)
        new_py = ani_reduce.simplify(py, error)
        new_pz = ani_reduce.simplify(pz, error)

        new_sx = ani_reduce.simplify(sx, error)
        new_sy = ani_reduce.simplify(sy, error)
        new_sz = ani_reduce.simplify(sz, error)

        new_qx = ani_reduce.simplify(qx, error)
        new_qy = ani_reduce.simplify(qy, error)
        new_qz = ani_reduce.simplify(qz, error)
        new_qw = ani_reduce.simplify(qw, error)

        curves = dict(px=new_px, py=new_py, pz=new_pz, sx=new_sx, sy=new_sy, sz=new_sz, qx=new_qx,
                      qy=new_qy, qz=new_qz, qw=new_qw)

        u_node = dict(curves=curves, name=node['name'])
        u_nodes.append(u_node)

    new_animation = dict(nodes=u_nodes, duration=animation['duration'],
                         version='unity', name=animation['name'])
    u_result = dict(animation=new_animation)

    # 上传oss
    local_path = './json/{}_animation.json'.format(datetime.now().timestamp())
    with open(local_path, 'w') as f:
        f.write(json.dumps(u_result))


def euler_to_quaternion(euler_rotation_list):
    import math

    """将欧拉角转换成四元数, 欧拉角是三个数的旋转角度列表
    :param euler_rotation_list: 三个数的欧拉角列表
    :return: 四元数组成的列表
    """
    (ex, ey, ez) = [(math.pi / 180 * 0.5 * angle) for angle in euler_rotation_list]
    sx = math.sin(ex)
    cx = math.cos(ex)
    sy = math.sin(ey)
    cy = math.cos(ey)
    sz = math.sin(ez)
    cz = math.cos(ez)
    qx = sx * cy * cz - cx * sy * sz
    qy = cx * sy * cz + sx * cy * sz
    qz = cx * cy * sz - sx * sy * cz
    qw = cx * cy * cz + sx * sy * sz
    return [qx, qy, qz, qw]


def playcanvs_euler_to_unity_quaternion(r):
    q = euler_to_quaternion(r)
    return [q[0], -q[1], -q[2], q[3]]


parse_playcanvs_to_gltf_animation("./json/origin2.json")
