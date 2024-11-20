# 将来の変更に備え設計する

## 関心の分離
依存関係を削減し、将来の変更を容易にするための最前かつ実証されている解は関心の分離です。
その核にある考え方は、機能をより小さい単位に分割、分離、切り出すことです。

関心の分離の目的は読解性を高めること、複雑さを軽減することにあ離、結果的にモジュールかを促進した設計になります。

単一責任の原則について: 
クラスを変更する理由は1つであるべきである。　とする考えかた。
意味する内容は、「真に属するものだけを同じグループにまとめること』です。
```
属す属さないを厳密に判断し、属さなければ別グループとするものです。
将来変更する際に、同じ原因で変更するものを１グループにまとめるという言い方もできます。このグループ分けにより、開発コードが持つ複数の異なるアスペクト間の人工的な関係性を削減でき（疎結合）将来の変更にも対象しやすくなります。上手にグループ分けすれば、1箇所を変更するだけで済むでしょう。
```

## 人工的な関係性の例
以下の抽象クラスDocumentを考える。

```C++
// include <some_json_library.h> // Potential physical dependency

class Document {
  public: 
    // ...
    virtual ~Document() = default;

    virtual void exportToJSON( /*...*/ ) const = 0;
    virtual void serialize(ByteStream&, /*...*/ ) const = 0;
    // ...
};
```
上の例はどんな種類のドキュメントにも使える、極めて有用な基底クラスに見える。
全派生クラスは都キュメントからJSONファイルを生成すべく、exportToJSON()関数を実装する必要があります。ドキュメント種別（e.g. PDF, Word）に関わらず、JSON形式にエクスポートできる。
serialize()関数により、ByteStreamを介し、Documentをバイト列へ変換できる。

上例は多数の依存関係を内包しているため、良くない設計だと言える。
ByteStreamクラスへの依存のように必要な依存もあるが、本来不要な人工的な依存関係が3つある。
2つはexportToJSON()関数により、もう1つはserialize()関数によりそれぞれ発生している。

```
派生クラスはexportToJSON()を必ず実装せねばならず、その際にサードパーティのJSONライブラリを利用するでしょう。
どのライブラリを選択しようとも、exportToJSON()がメンバ関数である以上、派生クラスは突如としてそのライブラリに依存することになります。そして恐らくは、一貫性という理由から全ての派生クラスが同じライブラリを採用するでしょう。
すると、全ての派生クラスが真に独立とはいえなくなり、設計上の判断であるJSONライブラリと人工的な関係性を持ってしまいます。また、特定のJSONライブラリに依存することにより、この階層構造はもはや軽量とは言えなくなり、再利用性が大きく制限される点も確実です。
別のライブラリに切り替えようとしても、全派生クラスが対応せねばならず、大掛かりな変更を強いられることになるでしょう。
```

serialize()関数にも同じような人工的依存関係が発生する。
serialize()の実装にもやはりサードパーティライブラリを利用すると思われる。

```
これにより依存関係の状況は著しく悪化します。
本来ならば関係を持たない二者（JSONエクスポートとシリアライゼーション）が交わり（直行し）、新たな関係性が発生するためです。
一方を変更すると、他方も変更せざるを得なくなる恐れがあります。

悪くするとexportToJSON()関数が、依存関係をもう1つ発生させる恐れがあります。JSONライブラリ実装詳細のなんらかの都合により、exportToJSON()コール時に渡す引数が影響を受けるかもしれません。この場合他のライブラリへ切り換えようとすると、exportToJSON()の関数シグニチャも変更せざるを得なくなってしまいます。すなわち、コール箇所全ての変更です。
```

3つ目の依存関係はserialize()関数により発生する。
この関数によってDocumentの派生クラスは、ドキュメントをどうシリアライズするかというグローバルな決定の影響下に置かれます。

ドキュメント種別を列挙方を用いて表現すると以下のようになる。
```C++

enum class DocumentType {
  pdf,
  word,
  // ... Potentially many more document types
};

// ドキュメント種別は増加する可能性がある
```

仮にDocumentクラスの実装内でこの列挙型を使用すると、期せずして全種別と関係を持ってしまうことになります。全ての派生クラスに、暗黙に他のドキュメント種別も全て見えているためです。その結果ドキュメント種別を変更すると既存のドキュメント種別全てに直接影響してしまいます。

```
このDocumentクラスは将来の変更が困難になる、SRP違反の見本のようなクラスです。無関係のアスペクトがさまざまに交わり、強い関係性を持ってしまっているため、Documentの派生クラスおよびユーザーはさまざまなことが原因で変更を強いられます。
```

## 論理的関係性と物理的関係性
関係性とは論理的な関係にとどまらず、物理的関係にも発展します。

DocumentがJSONライブラリやByteStreamに依存しているため、　Userも、アーキテクチャの最上位に位置するJSONライブラリとByteStreamに、間接的かつ推移的に依存しています。この依存関係は意図せずして、人工的に発生したもの（本来はUserはそれらに依存する必要はない。）
![](/ch1/1.ソフトウェア設計の重要性を理解する/guid2/before.drawio.png)

```
SRPが言っているのは、関心と、真に属さないもの、すなわち非凝集性（癒着性）のものとを、分離すべきということです。言い換えると、変更する原因が異なるものは**バリエーションポイント**に分離せよということです。
```
![](/ch1/1.ソフトウェア設計の重要性を理解する/guid2/after.drawio.png)

SRPに従い、Documentクラスおwリファクタリングする。
```C++
class Document {
  public:
    // ...
    virtual ~Document() = default;

    // ここにはexportToJSON()やserialize()は書かない。
    // 他のものとの強い結合を生じない、基本的なDocuumentの動作のみを書く
    // Only the very basic document operations, that do not cause strong coupling, remain
};
```

```
JSONとシリアライゼーションの両アスペクトは、Documentクラスの重要機能ではありません。Documentクラスは全種別のドキュメントの、非常に基本的な処理を表現しているに過ぎず、またそうあるべきです。直行するアスペクトは全て分離すべきであり、これにより変更しやすさが著しく向上します。
```

## DRY原則
商品の階層構造
![](/ch1/1.ソフトウェア設計の重要性を理解する/guid2/item.drawio.png)
階層構造の最上位にはItem基底クラスがある。
```C++
// -----<Money.h>-----
class Money { /*...*/ };

Money operator*(Money money, double facotr);
Money operator+(Money lhs, Money rhs);

// -----<Item.h>-----
#include <Money.h>

class Item {
  public: 
    virtual ~Item() = default;
    virtual Money price() const = 0;
};
```

CppBookを考える。
```C++
// -----<CppBook.h>-----
#include <Item.h>
#include <Money.h>
#include <string>

class CppBook : public Item {
  public: 
    explicit CppBook(std::string title, std::string author, Money price)
      : title_(std::move(title))
      , author_(std::move(author))
      , pricewithTax_(price * 1.15) // 15% tax
    {}

    std::string const& title() const { return title_; }
    std::string const& author() const { return author_; }

    Money price() const override { return pricewithTax_; }
  
  private:
    std::string title_;
    std::string author_;
    Money pricewithTax_;
};
```

ConferenceTicketを考える。
```C++
// -----<ConferenceTicket.h>-----
#include <Item.h>
#include <Money.h>
#include <string>

class ConferenceTicket : public Item {
  public:
    explicit ConferenceTicket(std::string name, Money price)
     : name_(std::move(name))
     , priceWithTax_(price * 1.15) // 15% tax
    {}

    std::string const& name() const { return name_; } 

    Money price() const override { return priceWithTax_; }
  
  private:
    std::string name_;
    Money priceWithTax_;
}
```

CppBookにもConferenceTicketにも同じように価格に課税される。

商品別に個別に価格を指定できるため、SRPを遵守していて、バリエーションポイントを切り出し、独立させているように見える。
ただし、現状では、税率変更の影響はItemクラスから派生した全クラスに及ぶ。

```
SRPがバリエーションポイントの分離をいうように、開発コード全体を通じ情報が重複しないよう注意すべきです。全ての場面で単一責任を実現すべきであり、個々の責任はシステム内で1箇所にしか存在しないようにすべきです。
この考え方を一般に**DRY原則**といいます。重要な情報を複数箇所に記述してはならない　ー　変更が1箇所に済むようなシステムを設定しなさい という意味です。
理想的には、税率が１箇所にしか登場せず変更が容易であるべきです。
```

## 時期尚早な関心の分離
```
SRPもDRYも保守性を高め変更を容易にする、優れた道具です。しかし道具は最終目的ではありません。
将来どんな変更性が発生するか明確な考えがないままソフトウェアエンティティを分離してしまっては、生産性を大きく損ないます。

どんな種類の変更が発生するかが明確になるまでまち、それから変更が可能な限り容易になるよう、リファクタリングすれば良いのです。
* 変更しやすさには、期待される動作が変更後に影響を受けていないことを確認する、単体テストが必ず含まれる点を忘れたはいけない。
```

# ガイドライン２の要約
- ソフトウェアに変更はつきものと認識する。
- 将来の変更に備えた設計とし、ソフトウェアの順応性を確保する。
- 無用の関係性を排除すべく、無関係の直行するアスペクトの結合を避ける
- 関係性が増加すると変更が連なりやすくなり、将来の変更が困難になることを理解する。
- 単一責任の原則に従い、関心を分離する。
- DRY原則に従い、重複を最小化する。
- 将来の変更が確実でなければ、時期尚早な抽象化を避ける。