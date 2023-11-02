from flask import Flask, render_template, make_response, request, redirect, url_for, send_from_directory, flash
from flask_sqlalchemy import SQLAlchemy
import hashlib
import random
import os
from decimal import Decimal

app = Flask(__name__)

# 请先创建数据库！

# 设置连接数据库的URL
app.config['SQLALCHEMY_DATABASE_URI'] = 'mysql://root:114514@127.0.0.1:3306/lab3'
# 设置每次请求结束后会自动提交数据库中的改动
app.config['SQLALCHEMY_COMMIT_ON_TEARDOWN'] = True
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = True
# 查询时会显示原始SQL语句
app.config['SQLALCHEMY_ECHO'] = True
# 配置密钥
app.config['SECRET_KEY'] = os.urandom(24)

db = SQLAlchemy(app)

class Admin(db.Model):
    # 表名
    __tablename__ = 'admin'
    # 属性
    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    name = db.Column(db.String(20), nullable=False)
    md5 = db.Column(db.String(32), nullable=False)
    # 关系
    goods = db.relationship('Goods', backref='admin')
    complaint = db.relationship('Complaint', backref='admin')

class CartItem(db.Model):
    # 表名
    __tablename__ = 'cartItem'
    # 属性
    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    # 外键
    goods_id = db.Column(db.Integer, db.ForeignKey('goods.id'), nullable=False)
    user_id = db.Column(db.Integer, db.ForeignKey('user.id'), nullable=False)

class Complaint(db.Model):
    # 表名
    __tablename__ = 'complaint'
    # 属性
    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    content = db.Column(db.String(1000), nullable=False)
    status = db.Column(db.String(20), nullable=False)
    # 外键
    order_id = db.Column(db.Integer, db.ForeignKey('orders.id'))
    admin_id = db.Column(db.Integer, db.ForeignKey('admin.id'))
    complaintType_id = db.Column(db.Integer, db.ForeignKey('complaintType.id'), nullable=False)

class ComplaintType(db.Model):
    # 表名
    __tablename__ = 'complaintType'
    # 属性
    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    name = db.Column(db.String(20), nullable=False)
    # 关系
    complaint = db.relationship('Complaint', backref='complaintType')

class Goods(db.Model):
    # 表名
    __tablename__ = 'goods'
    # 属性
    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    name = db.Column(db.String(40), nullable=False)
    position = db.Column(db.String(60), nullable=False)
    state = db.Column(db.String(10), nullable=False)
    price = db.Column(db.DECIMAL(8,2), nullable=False)
    old = db.Column(db.String(10))
    image = db.Column(db.String(100))
    # 外键
    goodsType_id = db.Column(db.Integer, db.ForeignKey('goodsType.id'), nullable=False)
    admin_id = db.Column(db.Integer, db.ForeignKey('admin.id'))
    user_id = db.Column(db.Integer, db.ForeignKey('user.id'), nullable=False)
    # 关系
    orders = db.relationship('Orders', backref='goods', uselist=False)
    cartItem = db.relationship('CartItem', backref='goods')

class GoodsType(db.Model):
    # 表名
    __tablename__ = 'goodsType'
    # 属性
    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    name = db.Column(db.String(20), nullable=False)
    # 关系
    goods = db.relationship('Goods', backref='goodsType')

class Orders(db.Model):
    # 表名
    __tablename__ = 'orders'
    # 属性
    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    state = db.Column(db.String(20), nullable=False)
    time = db.Column(db.TIMESTAMP, nullable=False)
    # 外键
    user_id = db.Column(db.Integer, db.ForeignKey('user.id'), nullable=False)
    goods_id = db.Column(db.Integer, db.ForeignKey('goods.id'), nullable=False)
    # 关系
    complaint = db.relationship('Complaint', backref='orders', uselist=False)

class User(db.Model):
    # 表名
    __tablename__ = 'user'
    # 属性
    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    name = db.Column(db.String(20), nullable=False)
    md5 = db.Column(db.String(32), nullable=False)
    balance = db.Column(db.DECIMAL(8,2), nullable=False)
    # 关系
    goods = db.relationship('Goods', backref='user')
    orders = db.relationship('Orders', backref='user')
    cartItem = db.relationship('CartItem', backref='user')
    # 索引
    ix_name = db.Index('ix_name', name)

# 触发器：当生成订单时，自动修改物品状态为“已售出”
create_trigger = """
CREATE TRIGGER `update_goods_state` 
AFTER INSERT ON `orders` 
FOR EACH ROW
BEGIN
    UPDATE goods SET state = '已售出' WHERE id = NEW.goods_id;
END
"""

# 视图：获取所有购物车物品
drop_view = "DROP VIEW IF EXISTS get_cart_items;"
get_cart_items = """
CREATE VIEW get_cart_items AS
SELECT goods.id, goods.name, goods.position, goods.state, goods.price, goods.old, goods.image, goods.goodsType_id, goods.admin_id, goods.user_id
FROM goods, cartItem, user
WHERE goods.id = cartItem.goods_id AND cartItem.user_id = user.id;
"""
query_view = "SELECT * FROM get_cart_items;"

# 计算某user用户车中的物品总价
drop_fuction = "DROP FUNCTION IF EXISTS `get_cart_total`;"
store_function = """
CREATE FUNCTION `get_cart_total`(user_id INT) RETURNS decimal(8,2)
READS SQL DATA
BEGIN
    DECLARE total decimal(8,2);
    SELECT SUM(price) INTO total FROM get_cart_items WHERE user_id = user_id;
    RETURN total;
END
"""

def initdb():
    with app.app_context():
        # 初始化数据库
        db.drop_all()
        db.create_all()
        # 设置触发器
        db.session.execute(db.text(create_trigger))
        # 设置视图
        db.session.execute(db.text(drop_view))
        db.session.execute(db.text(get_cart_items))
        # 设置存储函数
        db.session.execute(db.text(drop_fuction))
        db.session.execute(db.text(store_function))
        # 添加测试用户
        db.session.add(User(name="test", md5="098f6bcd4621d373cade4e832627b4f6", balance=100000.00))
        user = User.query.filter_by(name="test").first()
        # 添加管理员
        db.session.add(Admin(name="admin", md5="098f6bcd4621d373cade4e832627b4f6"))
        admin = Admin.query.filter_by(name="admin").first()
        # 添加物品类型
        db.session.add(GoodsType(name="电子产品"))
        db.session.add(GoodsType(name="书籍"))
        elec = GoodsType.query.filter_by(name="电子产品").first()
        book = GoodsType.query.filter_by(name="书籍").first()
        # 添加二手物品
        db.session.add(Goods(name="C++ Primer", position="T2教学楼", state="待审核", price=15.3, old="伊拉克成色", goodsType=book, user=user, image="images/c++primer.jpg"))
        db.session.add(Goods(name="4090ti", position="荔园7栋", state="在售", price=21999.99, old="全新", goodsType=elec, user=user, image="images/4090ti.png"))
        db.session.add(Goods(name="余华的《活着》", position="荔园3栋", state="在售", price=21999.99, old="可接受", goodsType=book, user=user, image="images/tolive.jpg"))
        # 添加投诉类型
        db.session.add(ComplaintType(name="虚假宣传"))
        db.session.add(ComplaintType(name="不发货"))
        # 提交
        db.session.commit()

userTokens = {}
adminTokens = {}

@app.route("/")
def index():
    return render_template("index.html")

@app.route("/logout")
def logout():
    # 删除用户token
    token = request.cookies.get("userToken")
    if userTokens.get(token):
        del userTokens[token]
    if token:
        resp = make_response(redirect(url_for("index")))
        resp.delete_cookie("userToken")
        return resp
    return redirect(url_for("index"))

@app.route("/adminlogout")
def adminlogout():
    # 删除管理员token
    token = request.cookies.get("adminToken")
    if adminTokens.get(token):
        del adminTokens[token]
    if token:
        resp = make_response(redirect(url_for("index")))
        resp.delete_cookie("adminToken")
        return resp
    return redirect(url_for("index"))

@app.route("/login", methods=["GET", "POST"])
def login():
    # 检查token
    token = request.cookies.get("userToken")
    if token:
        if userTokens.get(token):
            return redirect(url_for("main"))    # 已登录
        else:
            resp = make_response(redirect(url_for("login")))
            resp.delete_cookie("userToken")
            return resp                         # 清除无效token
    # 处理注册登录请求
    if request.method == "POST":
        if request.form.get("username"):
            # 用户登录
            username = request.form["username"]
            password = request.form["password"]
            md5 = hashlib.md5()
            md5.update(password.encode("utf-8"))
            print(f"\tlogin: user({username}), password({password})")
            print(f"\tmd5({md5.hexdigest()})\n")
            # 数据库操作
            with app.app_context():
                user = User.query.filter_by(name=username).first()
                if not user:
                    # 用户名不存在
                    print("\tuser does not exist")
                    return redirect(url_for("login", info="用户名或密码错误！"))
                elif user.md5 != md5.hexdigest():
                    # 密码错误
                    print("\tincorrect password")
                    return redirect(url_for("login", info="用户名或密码错误！"))
                else:
                    # 登录成功
                    randomToken = str(random.randint(pow(2, 63), pow(2, 64) - 1))
                    print(f"\tlogin: user({username}) token({randomToken})")
                    userTokens[str(randomToken)] = username
                    resp = make_response(redirect(url_for("main")))
                    resp.set_cookie("userToken", str(randomToken))
                    return resp
        elif request.form.get("newUsername"):
            # 用户注册
            newUsername = request.form["newUsername"]
            newPassword = request.form["newPassword"]
            md5 = hashlib.md5()
            md5.update(newPassword.encode("utf-8"))
            print(f"\tregister: user({newUsername}), password({newPassword})")
            print(f"\tmd5({md5.hexdigest()})\n")
            # 数据库操作
            with app.app_context():
                if User.query.filter_by(name=newUsername).first():
                    # 用户名已存在
                    print("\tuser already exists")
                    return redirect(url_for("login", info="用户已存在，请直接登录！"))
                else:
                    print(f"register: user {newUsername}")
                    db.session.add(User(name=newUsername, md5=md5.hexdigest(), balance=1000.00))
                    db.session.commit()
                    randomToken = random.randint(pow(2, 63), pow(2, 64) - 1)
                    userTokens[str(randomToken)] = newUsername
                    resp = make_response(redirect(main))
                    resp.set_cookie("adminToken", str(randomToken))
                    return resp
    return render_template("login.html", info=request.args.get("info"))

@app.route("/adminlogin", methods=["GET", "POST"])
def adminlogin():
    token = request.cookies.get("adminToken")
    if token and adminTokens.get(token):
        # 已登录
        return redirect(url_for("main"))
    if request.method == "POST":
        if request.form.get("username"):
            # 用户登录
            username = request.form["username"]
            password = request.form["password"]
            md5 = hashlib.md5()
            md5.update(password.encode("utf-8"))
            print(f"\tadmin login: admin({username}), password({password})")
            print(f"\tmd5({md5.hexdigest()})\n")
            # 数据库操作
            with app.app_context():
                user = Admin.query.filter_by(name=username).first()
                if not user:
                    # 用户名不存在
                    print("\tadmin does not exist")
                    return redirect(url_for("adminlogin", info="用户名或密码错误"))
                elif user.md5 != md5.hexdigest():
                    # 密码错误
                    print("\tincorrect password")
                    return redirect(url_for("adminlogin", info="用户名或密码错误"))
                else:
                    # 登录成功
                    randomToken = str(random.randint(pow(2, 63), pow(2, 64) - 1))
                    print(f"\tlogin: user({username}) token({randomToken})")
                    adminTokens[str(randomToken)] = username
                    resp = make_response(redirect(url_for("adminmain")))
                    resp.set_cookie("adminToken", str(randomToken))
                    return resp
    return render_template("adminlogin.html")

@app.route('/images/<filename>')
def getimage(filename):
    return send_from_directory('images', filename)

@app.route("/main")
def main():
    token = request.cookies.get("userToken")
    if token:
        if userTokens.get(token):   # 已登录
            # 查询物品
            with app.app_context():
                # 需要排除已经加入购物车的物品
                user = User.query.filter_by(name=userTokens.get(token)).first()
                goods_list = Goods.query.filter( db.and_( 
                    Goods.cartItem.any(user_id=user.id) == False, \
                    Goods.state == "在售") ).all()
                return render_template("main.html", user=user, goods_list=goods_list) 
        else:
            resp = make_response(redirect(url_for("login")))
            resp.delete_cookie("userToken")
            return resp
    else:                                   # 未登录
        return redirect(url_for("login"))

@app.route("/adminmain")
def adminmain():
    token = request.cookies.get("adminToken")
    if token:
        if adminTokens.get(token):
            with app.app_context():
                goods_list = Goods.query.filter_by(state="待审核").all()
                return render_template("adminmain.html", username=adminTokens[token], goods_list=goods_list)
        else:
            resp = make_response(redirect(url_for("adminlogin")))
            resp.delete_cookie("adminToken")
            return resp
    else:
        return redirect(url_for("adminlogin"))

@app.route("/addcart/<goods_id>")
def addcart(goods_id):
    token = request.cookies.get("userToken")
    if token:
        if userTokens.get(token):
            with app.app_context():
                user = User.query.filter_by(name=userTokens.get(token)).first()
                goods = Goods.query.filter_by(id=int(goods_id)).first()
                db.session.add(CartItem(goods=goods, user=user))
                db.session.commit()
            flash("添加购物车成功！")
            return redirect(url_for("main"))
        else:
            resp = make_response(redirect(url_for("login")))
            resp.delete_cookie("userToken")
            return resp
    else:
        return redirect(url_for("login"))

@app.route("/removecart/<goods_id>")
def removecart(goods_id):
    token = request.cookies.get("userToken")
    if token:
        if userTokens.get(token):
            with app.app_context():
                user = User.query.filter_by(name=userTokens.get(token)).first()
                db.session.delete(CartItem.query.filter_by(user_id=user.id, goods_id=goods_id).first())
                db.session.commit()
            flash("商品已移出购物车！")
            return redirect(url_for("cart"))
        else:
            resp = make_response(redirect(url_for("login")))
            resp.delete_cookie("userToken")
            return resp
    else:
        return redirect(url_for("login"))

@app.route("/release", methods=["GET", "POST"])
def release():
    token = request.cookies.get("userToken")
    if token:
        username = userTokens.get(token)
        if username:           # 已登录
            if request.method == "POST":
                # 保存图片
                Image = request.files["itemImage"]
                if not os.path.exists("images"):
                    os.makedirs("images")
                file_path = os.path.join("images", Image.filename)
                Image.save(file_path)
                # 处理发布请求
                name = request.form["itemName"]
                position = request.form["itemPosition"]
                price = request.form["itemPrice"]
                old = request.form["itemOld"]
                goodsType = request.form["itemType"]
                print(f"\trelease: name({name}), position({position}), price({price}), old({old}), goodsType({goodsType})")
                # 数据库操作
                with app.app_context():
                    user = User.query.filter_by(name=username).first()
                    goodsType = GoodsType.query.filter_by(name=goodsType).first()
                    db.session.add(Goods(name=name, position=position, state="待审核", price=price, old=old, goodsType=goodsType, user=user, image=file_path))
                    db.session.commit()
                flash("发布成功！请等待管理员审核")
                return redirect(url_for("main"))
            # 首次访问
            return render_template("release.html", username=userTokens.get(token))
        else:
            resp = make_response(redirect(url_for("login")))
            resp.delete_cookie("userToken")
            return resp
    else:                                   # 未登录
        return redirect(url_for("login"))

@app.route("/mygoods")
def mygoods():
    token = request.cookies.get("userToken")
    if token:
        username = userTokens.get(token)
        if username:
            with app.app_context():
                user = User.query.filter_by(name=username).first()
                goods_list = Goods.query.filter(db.and_(Goods.user_id == user.id, Goods.state != "已删除")).all()
                return render_template("mygoods.html", username=username, goods_list=goods_list)
        else:
            resp = make_response(redirect(url_for("login")))
            resp.delete_cookie("userToken")
            return resp
    else:
        return redirect(url_for("login"))

@app.route("/removegoods/<goods_id>")
def removegoods(goods_id):
    token = request.cookies.get("userToken")
    if token:
        username = userTokens.get(token)
        if username:
            with app.app_context():
                user = User.query.filter_by(name=username).first()
                # 把物品状态改为“已删除”
                goods = Goods.query.filter_by(id=int(goods_id)).first()
                goods.state = "已删除"
                db.session.commit()
                flash("商品已删除！")
                return redirect(url_for("mygoods"))
        else:
            resp = make_response(redirect(url_for("login")))
            resp.delete_cookie("userToken")
            return resp
    else:
        return redirect(url_for("login"))

@app.route("/cart")
def cart():
    token = request.cookies.get("userToken")
    if token:
        username = userTokens.get(token)
        if username:
            with app.app_context():
                user = User.query.filter_by(name=username).first()
                goods_list = Goods.query.filter(Goods.cartItem.any(user_id=user.id) == True).all()
                return render_template("cart.html", user=user, goods_list=goods_list)
        else:
            resp = make_response(redirect(url_for("login")))
            resp.delete_cookie("userToken")
            return resp
    else:
        return redirect(url_for("login"))

@app.route("/settle/<goods_id>")
def settle(goods_id):
    token = request.cookies.get("userToken")
    if token:
        username = userTokens.get(token)
        if username:
            with app.app_context():
                user = User.query.filter_by(name=username).first()
                goods = Goods.query.filter_by(id=int(goods_id)).first()
                # 检查余额是否足够
                if user.balance < goods.price:
                    flash("余额不足！")
                    return redirect(url_for("cart"))
                # 扣减余额
                user.balance -= goods.price
                # 生成订单
                db.session.add(Orders(state="成交", time=db.func.now(), user=user, goods=goods))
                # 删除购物车物品
                db.session.delete(CartItem.query.filter_by(user_id=user.id, goods_id=goods_id).first())
                db.session.commit()
                flash("结算成功！")
                return redirect(url_for("cart"))
        else:
            resp = make_response(redirect(url_for("login")))
            resp.delete_cookie("userToken")
            return resp
    else:
        return redirect(url_for("login"))

@app.route("/myorders")
def myorders():
    token = request.cookies.get("userToken")
    if token:
        username = userTokens.get(token)
        if username:
            with app.app_context():
                user = User.query.filter_by(name=username).first()
                orders_list = Orders.query.filter_by(user_id=user.id).all()
                return render_template("myorders.html", user=user, orders_list=orders_list)
        else:
            resp = make_response(redirect(url_for("login")))
            resp.delete_cookie("userToken")
            return resp
    else:
        return redirect(url_for("login"))

@app.route("/complaint/<orders_id>", methods=["GET", "POST"])
def complaint(orders_id):
    token = request.cookies.get("userToken")
    if token:
        username = userTokens.get(token)
        if username:
            if request.method == "POST":
                # 处理投诉请求
                content = request.form["itemContent"]
                complaintType = request.form["itemType"]
                print(f"\tcomplaint: content({content}), complaintType({complaintType})")
                # 数据库操作
                with app.app_context():
                    user = User.query.filter_by(name=userTokens.get(request.cookies.get("userToken"))).first()
                    complaintType = ComplaintType.query.filter_by(name=complaintType).first()
                    orders = Orders.query.filter_by(id=orders_id).first()
                    db.session.add(Complaint(content=content, status="待处理", complaintType=complaintType, orders=orders))
                    orders.state = "已投诉"
                    db.session.commit()
                flash("投诉成功！请等待管理员处理")
                return redirect(url_for("myorders"))
            with app.app_context():
                user = User.query.filter_by(name=username).first()
                orders = Orders.query.filter_by(id=orders_id).first()
                return render_template("complaint.html", user=user, orders=orders)
        else:
            resp = make_response(redirect(url_for("login")))
            resp.delete_cookie("userToken")
            return resp

@app.route("/recharge", methods=["GET", "POST"])
def recharge():
    token = request.cookies.get("userToken")
    if token:
        username = userTokens.get(token)
        if username:
            with app.app_context():
                user = User.query.filter_by(name=username).first()
                if request.method == "POST":
                    # 处理充值请求
                    amount = request.form["itemAmount"]
                    print(f"\trecharge: amount({amount})")
                    # 数据库操作
                    user.balance += Decimal(amount)
                    db.session.commit()
                    flash("充值成功！")
                    return redirect(url_for("main"))
                return render_template("recharge.html", user=user)
        else:
            resp = make_response(redirect(url_for("login")))
            resp.delete_cookie("userToken")
            return resp
    else:
        return redirect(url_for("login"))

@app.route("/accept/<goods_id>")
def accept(goods_id):
    token = request.cookies.get("adminToken")
    if token:
        username = adminTokens.get(token)
        if username:
            with app.app_context():
                goods = Goods.query.filter_by(id=int(goods_id)).first()
                goods.state = "在售"
                db.session.commit()
                return redirect(url_for("adminmain"))
        else:
            resp = make_response(redirect(url_for("adminlogin")))
            resp.delete_cookie("adminToken")
            return resp
    else:
        return redirect(url_for("adminlogin"))

@app.route("/reject/<goods_id>")
def reject(goods_id):
    token = request.cookies.get("adminToken")
    if token:
        username = adminTokens.get(token)
        if username:
            with app.app_context():
                goods = Goods.query.filter_by(id=int(goods_id)).first()
                goods.state = "审核未通过"
                db.session.commit()
                return redirect(url_for("adminmain"))
        else:
            resp = make_response(redirect(url_for("adminlogin")))
            resp.delete_cookie("adminToken")
            return resp
    else:
        return redirect(url_for("adminlogin"))

@app.route("/admincomplaint")
def admincomplaint():
    token = request.cookies.get("adminToken")
    if token:
        username = adminTokens.get(token)
        if username:
            with app.app_context():
                user = User.query.filter_by(name=username).first()
                complaint_list = Complaint.query.filter_by(status="待处理").all()
                return render_template("admincomplaint.html", user=user, complaint_list=complaint_list)
        else:
            resp = make_response(redirect(url_for("adminlogin")))
            resp.delete_cookie("adminToken")
            return resp
    else:
        return redirect(url_for("adminlogin"))

@app.route("/refund/<complaint_id>")
def refund(complaint_id):
    token = request.cookies.get("adminToken")
    if token:
        username = adminTokens.get(token)
        if username:
            with app.app_context():
                complaint = Complaint.query.filter_by(id=int(complaint_id)).first()
                complaint.status = "已处理"
                complaint.orders.state = "退款"
                complaint.orders.user.balance += complaint.orders.goods.price
                db.session.commit()
                return redirect(url_for("admincomplaint"))
        else:
            resp = make_response(redirect(url_for("adminlogin")))
            resp.delete_cookie("adminToken")
            return resp
    else:
        return redirect(url_for("adminlogin"))

@app.route("/norefund/<complaint_id>")
def norefund(complaint_id):
    token = request.cookies.get("adminToken")
    if token:
        username = adminTokens.get(token)
        if username:
            with app.app_context():
                complaint = Complaint.query.filter_by(id=int(complaint_id)).first()
                complaint.status = "已处理"
                complaint.orders.state = "投诉失败"
                db.session.commit()
                return redirect(url_for("admincomplaint"))
        else:
            resp = make_response(redirect(url_for("adminlogin")))
            resp.delete_cookie("adminToken")
            return resp
    else:
        return redirect(url_for("adminlogin"))

if __name__ == '__main__':
    initdb()
    app.run(debug=True)