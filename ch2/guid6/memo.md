# 抽象化から期待される動作に従う

ソフトウェア設計の重要な要素に、ソフトウェアエンティティの分離があります。その鍵となるのが抽象化です。

## 期待される動作に従わない例
Rectangle基底クラスを考える。

```C++
class Rectangle {
  public:
    // ...
    virtual ~Rectangle() = default;

    int getWidth() const; 
    int getHeight() const;

    virtual void setWidth(int);
    virtual void setHeight(int);

    virtual int getArea() const;
    // ...
  private:
    int width;
    int height;
};
```
上例について、
- 仮想デストラクタがあるため、Rectangleは基底クラスとして使われることを想定している。
- Rectangleは`width`と`height`の2つのメンバ変数をもつ。　また、getterとsetterがある。settterでは個別に値が設定でき、widthを変更してもheightは変更されない。
- getArea()メンバ関数をもち、getArea()は面積の計算をして、戻り値をして返す。

今度はSquareクラスを考える。
```C++
class Square : public Rectangle {
  public:
    // ...
    void setWidth(int) override;
    void setHeight(int) override;
    // ...
};
```

上例のSquareクラスはRectangleクラスのpublicな派生クラスです。幾何学的に言って、正方形は長方形の一例であるため、妥当な派生です。

Squareには辺の長さが1種類しかないという特徴があります。しかし、Rectangle基底クラスはwidthと　heightの2種類の辺の長さを持ちます。そのため、Squareがその不変条件（invariants）を常に維持するよう保障しなければなりません。
上例の実装にはメンバ変数が2つ、getter関数も2つある以上、両メンバ変数が常に同じ値になる保障が必要です。そこで、setWidth()メンバ関数をオーバーライドして、widthとheightの両方に値を代入するようにします。setHeight()も同様です。

上例の2クラスを使ってみて、形の異なる長方形へ変形する関数を考えます。
```C++
void transform(Rectangle& rectangle) {
  rectangle.setWidth(7);
  rectangle.setHeight(4);

  assert(rectangle.getArea() == 28);
}
```

上例のtransform()関数は、引数に非constな参照型のRectangleオブジェクトを受け取ります。
transform()関数は、引数のRectangleオブジェクトの幅と高さを変更し、その面積が28になることを確認します。

main() 関数からコールしてみます。
```C++  
int main() {
  Square s{};
  s.setWidth(5);

  transform(s);

  return EXIT_SUCCESS;
}
```

上例のmain()関数では、長方形の一例であるSquareを作成します。Squareの参照はRectangleの参照に暗黙的に型変換されるため、Squareでも問題なくtransform() 関数へ渡すことができます。

このコードは、transformの中のassert文で失敗します。
この異常終了はなぜ起こったでしょうか？このtransform()関数は長方形の幅と高さを個別に変更可能と想定しており、実際にsetWidth()とsetHeight()で明示的に変更しています。しかし想定外だったのは、Squareクラスのような幅と高さを個別に変更できない特殊な長方形が存在することです。
抽象化でのこのような想定外の存在は、そのままLSPからの逸脱になります。

## リスコフの置換原則
LSPは抽象化が期待する動作に関するもので、これを満たす派生クラスを**振る舞いサブタイピング**といいます。
```
サブタイプの要件: φ(x)を型Tのオブジェクトxについて、証明可能な性質とする。
               このとき、φ(y)は型TのサブタイプSのオブジェクトyについて、真でなければならない。
```
この原則が定義する内容を、一般に**IS-A関係**と言います。サブタイプはこの関係、すなわち抽象化が要件とする内容を**遵守しなければなりません**。
次のような性質があります。

### サブタイプでは事前条件を強化してはならない。
  サブタイプは、スーパータイプが実現する以上の内容を関数に期待してはならない。抽象化が期待する内容から逸脱するためである。

```C++
srtuct X {
  virtual ~X() = default;

  // Prediction: the function accepts all `i` greater than 0
  // 事前条件: 関数は1以上のすべての`i`を受け入れる
  virtual void f(int i) const {
    assert(i > 0);
  }
};

struct Y : public X {
  // Prediction: the function accepts all `i` greater than 10
  // This would stregthen ehe precondition; 
  // numbers between 1 and 10 would no longer be allowed. This is a LSP violation.
  // 事前条件: 関数は11以上のすべての`i`を受け入れる。
  // これは事前条件の強化にあたり、1から10の数値は許可されなくなる。これはLSP違反である。
  void f(int i) const override {
    assert(i > 10);
  }
};
```

### サブタイプでは事後条件を弱めてはならない。
  サブタイプは、関数終了時にスーパータイプが保障する内容を弱めてはいけない。抽象化が期待する内容から逸脱するためである。

```C++
struct X {
  virtual ~X() = default;

  // Prediction: the function will only return values larger than 0
  // 事後条件: 関数は0より大きい値のみを返す
  virtual int f() const {
    int i; 
    // ...
    assert(i > 0);
    return i;
  }
};

struct Y : public X {
  // Prediction: the function will return any value.
  // This would weaken the postcondition; 
  // nagative numbers and 0 would be allowed. This is a LSP violation.
  // 事後条件: 関数は任意の値を返す。
  // これは事後条件の弱体化にあたり、負の数値と0も許可される。これはLSP違反である。
  int f() const override {
    int i; 
    // ...
    return i;
  }
};
```

## サブタイプの関数の戻り型は共変（covariance）でなければならない
  サブタイプのメンバ関数は、戻り型のサブタイプを返せる。C++では言語が直接対応する。ただし、戻り型のスーパータイプを返してはならない。

```C++
struct Base { /* ...some virtual functions, including destructor... */ };
struct Derived : public Base { /* ... */};

struct X {
  virtual ~X() = default;
  virtual Base* f();
};

struct Y : public X {
  Derived* f() override; // Covariant return type
};
```

## サブタイプの関数パラメータは反変(contravariance)でなければならない
  サブタイプのメンバ関数は、引数のスーパータイプを受け取れる。C++では言語の直接対応はしない。

```C++
struct Base { /* soe virtual functions, including destructor... */ };
struct Derived : public Base { /* ... */};

struct X {
  virtual ~X() = default;
  virtual void f(Derived* );
};

struct Y : public X {
  void f(Base* ); // Contravariant fuction parameter;
                  // Not supported in C++. Therefore the fuction does not override, but fails to compile.
                  // 反変な引数。C++ではサポートされていない。関数はオーバーライドできず、コンパイルエラーになる。
};
```

## スーパータイプの不変条件はサブタイプでも維持されなければならない　
  スーパータイプの状態に期待される内容は、サブタイプのものも含め、全てのメンバ関数のコール前後で常に有効でなければならない。

```C++
struct X {
  explicit X(int v = 1) 
    : value_(v) 
    {
      if(v < 1 || v > 10) throw std::invalid_argument(/*...*/);
    }

    virtual ~X() = default;

    int get() const { return value_; }
  
  protected:
    int value_; // Invariant: must be within the range [1..10]
};

struct Y : public X {
  public:
    Y() 
      : X()
      {
        value_ = 11; // Broken invariant: After the constructor, `value_`
                     // is out of expected range. 
      }
};
```

この例でRectangleが期待する内容とは、幅と高さを個別に変更できることです。別の言い方をすると、getWidth()の戻り値はsetHeight()をコールした後でも変化しない、となります。どんな長方形にも適用する、直感的にわかりやすい内容です。

しかし、Squareクラスは自身で、全ての辺の長さが常に等しいという不変条件を導入しています。この不変条件なくして、正方形は成り立ちません。ところが自らの不変条件を堅持すると、基底クラスが期待する内容から逸脱してしまいます。すなわちSquareクラスはRectangleクラスが期待する内容を満たしておらず、この階層構造はIS-A関係を表現できていません。そのためSquareは、Rectangleなら使用できる全ての場合では、使用できないのです。

正方形と長方形には幾何学的な関係性があるが、この例の継承関係は破綻しています。
幾何学的にはIS-A関係でも、LSPではIS-A関係にできません。

計算機科学で真に決定するのは、実際のインターフェース、すなわち基底クラスが期待する内容です。