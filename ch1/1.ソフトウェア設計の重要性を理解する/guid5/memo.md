# 拡張に備え設計する

## 開放/閉鎖原則

ガイドライン2のシリアライゼーションを例にする。
```C++
class Document {
  public: 
    // ...
    virtual ~Document() = default;

    virtual void serialize(ByteStream& bs, /*...*/) const = 0;
    // ...
};
```

seiralize()は純粋仮想関数なので、全ての派生クラスで実装しなければならない。

```C++
class PDF : public Document {
  public:
    // ...
    void serialize(ByteStream& bs, /*...*/) const override;\
};
```

serialize()メンバ関数はどう実装するのが良いでしょうか？
要件として、後でバイト列からPDFインスタンスを復元できること、というのがあります。（シリアライズ結果からの逆シリアライズ）
そのためには、バイト列が何を表現しているかの情報も無くめて持つことが必須です。
ガイドライン2では列挙型を使いました。
```C++
enum class DocumentType {
  pdf,
  word,
  // ... Potentially other document types
};
```
全派生クラスがこの列挙型を使い、バイト列の先頭にドキュメント種別を埋め込むことで、逆シリアライズが可能になります。
しかし、この設計は良くないと判断しました。（全ドキュメント派生クラスが他の種別を知ってしまい、全ドキュメント派生クラスに全種別に対する関係性が生まれる）

関係性を以下に図示します。
![](/ch1/1.ソフトウェア設計の重要性を理解する/guid5/step1.drawio.png)
**DocumentType列挙型を介した、異なるドキュメント種別間の人工的関係性**

上例の問題は、機能を拡張しようとした時に発覚します。
PDFとWord以外にplain XML形式に対応しようとした場合、理想的にはDocumentクラスから派生したXMLクラスを追加するだけで良いはずです。
しかし、この設計では、DocumentType列挙型にも対応が必要になります。
```C++
enum class DocumentType {
  pdf,
  word,
  xml, // The new type of document
  // ... Potentially other document types
}
```
上例の変更により、少なくとも、全ドキュメント種別を再コンパイルする必要があります。
さらに、他の開発者のコード拡張を著しく制限することになります。（誰でもDocumentType列挙型を変更できるわけではないため）
この関係性は単に間違っているというよりも、PDFとWordは本来新規追加されたXML形式のことなど、一切感知する必要はないのです。
なので、その存在を見ることも感じることもすべきではなく、再コンパイルしなければならないということは、もってのほかです。

この例の問題は、開放/閉鎖原則（OCP）に違反していることです。
```
ソフトウェアコンポーネント（クラス、モジュール、関数など）は、拡張に対してはオープンに、改造に対してクローズに、すべきである。
```
OCPが言っているのは、まず開発ソフトウェアは拡張可能であるべきだということです。拡張は容易であるべきで、理想的には単にコードを追加するだけで済むべきであり、（他コンポーネントの）既存のコードを改造するなどは、あるべきではないという意味です。（改造に対してはクローズ）

```
理論的には、拡張とは容易なはずです。新規派生クラスXMLを追加するのみで済むべきです。この新規クラスの追加が理由で、他のコードに改造などは発生しませんが、残念ながらserialize()関数は他のドキュメント種別と人工的な関係性を持ってしまっており、DocumentType列挙型に追加が発生します。この追加が、今度は他のDocumentにも影響してしまいます。まさにOCPの逆を行く状況です。
```

ここですべきことは、関心の分離です。
![](/ch1/1.ソフトウェア設計の重要性を理解する/guid5/step2.drawio.png)
**OCP違反を是正する関心の分離**
関心の分離、すなわち同じものに芯に属するものを求めることにより、異なるドキュメント種別間に意図せず発生した関係性を解消できます。
シリアライゼーションを扱うコードは、今やすべてSerializationコンポーネントに適切にグループ化され、論理的にアーキテクチャの別レベルに位置しています。
Seiralizationは全ドキュメント種別に依存しますが、逆にドキュメント種別はSerializationに依存しません。さらにどのドキュメントも他のドキュメント種別に一切関知しません。

- 列挙型はシリアライゼーションで使うから、（バイト列にドキュメント種別を埋め込むために）引き続き必要なのでは？
Serializationコンポーネントでは、DocumentType列挙型のようなものが必要なのは変わらない。
しかし、関心を分離したことにより、この依存関係問題は適切に解決できています。どのドキュメント種別も、もうDocumentType列挙型に依存していません。
依存関係を示す矢印は、全て下位（Serializationコンポーネント）から上位（ドキュメント種別: PDF, Word）へ向かっています。
この性質こそ、重要不可欠であり、正しく優れた設計なのです。

- ドキュメントの新規追加jに、Seiralizationコンポーネントに変更が必要なのでは？
新規ドキュメント種別を追加する場合、Serializationコンポーネントに変更が必要です。
これはOCPから逸脱していません。OCPはアーキテクチャ上で同じレベルや、上位の既存コードで変更を発生させるべきではないとするものですが、下位での変更発生を妨げるものではありません。
Seiralizationは**全ドキュメント種別に依存しなければならず**、また**全ドキュメント種別に対応しなければなりません**。そのため、Seiralizationはアーキテクチャ上で下位に位置しなければなりません。（**依存層**とみなす）

```
拡張性はソフトウェア設計時に意識的に検討する必要がある、またある決まった方法で開発ソフトウェアを拡張する必要が生じるということは、関心の分離の必要性を強く示唆しているの2点を如実に示しています。
開発ソフトウェアが将来どう拡張されるかを押さえておく、その**カスタマイゼーションポイント**を特定する、さらにその拡張を容易に行えるよう設計する の3点が肝要です。
```

## コンパイル時拡張性
標準ライブラリは拡張性を備えた設計となっています。
注目に値するのは、拡張性と言っても規定クラスを用いているわけではなく、関数オーバーロード、テンプレート、（クラス）テンプレートの特殊化を基盤としている点です。

関数オーバーロードによる拡張の良い例は、std::swap()アルゴリズムです。
C++11以降、std::swap()は次のように定義されています。
```C++
namespace std {
  template <typename T>
  void swap(T& a, T& b) {
    T tmp(std::move(a));
    a = std::move(b);
    b = std::move(tmp);
  }
} // namespace std
```

std::swap()は関数テンプレートとして定義されているため、任意の型で使用できます。
しかし、特に注意を要する型もあります。std::swap()では交換できない、もしくはするべきではないけど（効率的にmoveできないなどの理由から）、他の方法を用いれば効率的に交換可能な型です。そうはいっても、値型は交換可能であるべきです。
```
値型、もしくは値ライクな型は、noexceptな交換関数の提供を検討すべきである。
```
C++ Code Guidelines
https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines

そのような型を独自に定義したならば、std::swap()をオーバーロードする方法が使えます。
```C++
namespace custom {
  class CustomType {
    /* Implementation that requires a special form of swap */
  };

  void swap(CustomType& a, CustomType& b) {
    /* Special swap implementation for swapping two instances of CustomType */
  }
} // namespace custom
```

swap()を正しく使用すれば上例の専用関数がコールされ、CustomTypeの2インスタンスを交換できます。
```C++
template<typename T>
void some_function(T& value) {
  // ...
  T tmp(/*...*/);

  using std::swap;  // Enable the compiler to consider std::swap() for the subsequent swap() call
  swap(tmp, value); // Swap the two values; thanks to the unqualified call
                    // and thanks to ADL this would call `custom::swap()`
                    // in case `T` is `CustomType`
}
```

開発者が独自の型や動作を加えられるよう、std::swap()が**カスタマイゼーションポイント**として設計されているのは明らかです。
標準ライブラリの全てのアルゴリズムも同様です。std::find()とstd::find_if()を例に考えてみましょう。
```C++
template<typename InputIt, typename T>
constexpr InputIt find(InputIt first, InputIt last, T const& value);

template<typename InputIt, typename UnaryPredicate>
constexpr InputIt find_if(InputIt first, InputIt last, UnaryPredicate p);
```

テンプレート引数、また対応するコンセプトという形のおかげで、std::find()もstd::find_if()も暗黙に（他のアルゴリズムも全てそうですが）、検索に独自の（イテレータ）型を使用できます。
さらにstd::find_if()では、要素の比較方法もカスタマイズできます。これらの関数が拡張性やカスタマイズ性を意識して設計されたのは間違いありません。

**カスタマイゼーションポイント**で最後に挙げるのはテンプレートの特殊化の例です。このアプローチは、例えば、std::hashクラステンプレートで採用されています。
std::swap()の例で列挙したCustomTypeを再び例にすれば、次のようにstd::hashを明示的に特殊化できます。
```C++
template<>
struct std::hash<CustomType> {
  std::size_t operator() (customTYpe const& v) const noexcept {
    return /*...*/;
  }
}
```
std::hashは、その設計のおかげで、どんな型にも対応できます。最大の特徴は既存コードを修正する必要がない点です。個別に特殊化すれば、専用の要件に対応できます。
標準ライブラリのほぼ全てが、将来の拡張とカスタマイズに備えた設計になっています。標準ライブラリはアーキテクチャ上、最上位に位置しているため、他のどのコードにも一切依存していないのです。
開発コードは全て標準ライブラリに依存します。
