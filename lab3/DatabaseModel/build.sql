/*==============================================================*/
/* DBMS name:      MySQL 5.0                                    */
/* Created on:     2023/10/31 20:13:43                          */
/*==============================================================*/


/*==============================================================*/
/* Table: admin                                                 */
/*==============================================================*/
create table admin
(
   admin_id             int not null auto_increment  comment '',
   admin_name           varchar(20) not null  comment '',
   admin_md5            char(32) not null  comment '',
   primary key (admin_id)
);

/*==============================================================*/
/* Table: cartItem                                              */
/*==============================================================*/
create table cartItem
(
   cartItem_id          int not null auto_increment  comment '',
   goods_id             int not null  comment '',
   user_id              int not null  comment '',
   primary key (cartItem_id)
);

/*==============================================================*/
/* Table: complaint                                             */
/*==============================================================*/
create table complaint
(
   complaint_id         int not null auto_increment  comment '',
   order_id             int  comment '',
   admin_id             int  comment '',
   user_id              int not null  comment '',
   complaintType_id     int not null  comment '',
   complaint_content    varchar(1000) not null  comment '',
   status               varchar(20) not null  comment '',
   comlaint_id          varchar(1000)  comment '',
   primary key (complaint_id)
);

/*==============================================================*/
/* Table: complaintType                                         */
/*==============================================================*/
create table complaintType
(
   complaintType_id     int not null auto_increment  comment '',
   complaintType_name   varchar(20) not null  comment '',
   primary key (complaintType_id)
);

/*==============================================================*/
/* Table: goods                                                 */
/*==============================================================*/
create table goods
(
   goods_id             int not null auto_increment  comment '',
   goodsType_id         int not null  comment '',
   admin_id             int  comment '',
   user_id              int not null  comment '',
   goods_name           varchar(40) not null  comment '',
   goods_position       varchar(60) not null  comment '',
   goods_state          varchar(10) not null  comment '',
   goods_price          float(8,2) not null  comment '',
   goods_bought         smallint  comment '',
   goods_old            varchar(10)  comment '',
   primary key (goods_id)
);

/*==============================================================*/
/* Table: goodsType                                             */
/*==============================================================*/
create table goodsType
(
   goodsType_id         int not null auto_increment  comment '',
   goodsType_name       varchar(20) not null  comment '',
   primary key (goodsType_id)
);

/*==============================================================*/
/* Table: orders                                                */
/*==============================================================*/
create table orders
(
   order_id             int not null auto_increment  comment '',
   user_id              int not null  comment '',
   goods_id             int not null  comment '',
   order_state          varchar(20) not null  comment '',
   order_time           timestamp not null  comment '',
   primary key (order_id)
);

/*==============================================================*/
/* Table: user                                                  */
/*==============================================================*/
create table user
(
   user_id              int not null auto_increment  comment '',
   user_name            varchar(20) not null  comment '',
   user_md5             char(32) not null  comment '',
   user_balance         float(8,2) not null  comment '',
   primary key (user_id)
);

alter table cartItem add constraint FK_CARTITEM_CARTITEM__GOODS foreign key (goods_id)
      references goods (goods_id) on delete restrict on update restrict;

alter table cartItem add constraint FK_CARTITEM_USER_CART_USER foreign key (user_id)
      references user (user_id) on delete restrict on update restrict;

alter table complaint add constraint FK_COMPLAIN_ADMIN_COM_ADMIN foreign key (admin_id)
      references admin (admin_id) on delete restrict on update restrict;

alter table complaint add constraint FK_COMPLAIN_COMPLAINT_COMPLAIN foreign key (complaintType_id)
      references complaintType (complaintType_id) on delete restrict on update restrict;

alter table complaint add constraint FK_COMPLAIN_COMPLAINT_ORDERS foreign key (order_id)
      references orders (order_id) on delete restrict on update restrict;

alter table complaint add constraint FK_COMPLAIN_USER_COMP_USER foreign key (user_id)
      references user (user_id) on delete restrict on update restrict;

alter table goods add constraint FK_GOODS_ADMIN_GOO_ADMIN foreign key (admin_id)
      references admin (admin_id) on delete restrict on update restrict;

alter table goods add constraint FK_GOODS_GOODS_GOO_GOODSTYP foreign key (goodsType_id)
      references goodsType (goodsType_id) on delete restrict on update restrict;

alter table goods add constraint FK_GOODS_USER_GOOD_USER foreign key (user_id)
      references user (user_id) on delete restrict on update restrict;

alter table orders add constraint FK_ORDERS_ORDER_GOO_GOODS foreign key (goods_id)
      references goods (goods_id) on delete restrict on update restrict;

alter table orders add constraint FK_ORDERS_USER_ORDE_USER foreign key (user_id)
      references user (user_id) on delete restrict on update restrict;
