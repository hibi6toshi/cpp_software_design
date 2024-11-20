# インターフェースを分離し人工的関係性を排除する

## インターフェースを分離し関心の分離を図る
Documentクラスを考える。

```C++
class Document {
  public: 
    // ...
    virtual ~Document() = default;

    virtual void exportToJSON( /*...*/ ) const = 0;
    virtual void serialize(ByteStream&, /*...*/ ) const = 0;
    // ...
};
```

次のコードを考える
```C++
viod exportDocument(Document const& doc) {
  // ...
  doc.exportToJSON( /* pass necessary arguments */);
  // ...
}
```

上記のexportDocument()関数は渡されたドキュメントをJSON形式へエクスポートするだけで、シリアライゼーションやDocumentが備える他のアスペクトに**一切関知していません**。それでいながらDocumentの直行する多くのアスペクトを一まとめにするインターフェース定義のせいで、exportDocument()関数はJSONエクスポート以外のものにも依存してしまうのです。

この関係性の原因は**インターフェース分離の原則**からの逸脱にある。
本書の例で言えば、JSONエクスポートとシリアライゼーションの直行するアスペクトは分離すべきインターフェースです。
```C++
class JSONExportable {
  public:
    // ...
    virtual ~JSONExportable() = default;

    virtual void exportToJSON(/*...*/) const = 0;
    // ...
};

class Serializable {
  public:
    // ...
    virtual ~Serializable() = default;

    virtual void serialize(ByteStream& bs, /*...*/) const = 0;
    // ...
};

class Document 
  : public JSONExportable
  , public Serializable {
  public:
    // ...
    void exportToJSON(/*...*/) const override {
        // ...
    }

    void serialize(ByteStream& bs, /*...*/) const override {
        // ...
    }
    // ...
};
```
上記の関心の分離により、依存関係を実際に必要な関数セットにまで最小化できる。
```C++
void exportDocument(JSONExportable const& exportable) {
    // ...
    exportable.exportToJSON(/* pass necessary arguments */);
    // ...
}
```
上記のexportDocument()関数は分離されたJSONExportableインターフェースにのみ依存しており、シリアライゼーションには依存しなくなりました。ByteStreamクラスにはもう依存しておらず、疎結合にできた。

## テンプレート引数の要件は最小限に
インタフェースにより発生した依存関係を最小化するという基本的な考え型は、OOPに限らず、テンプレートにも適用できます。

```C++
template<typename InputIt, typename OutputIt>
Output copy(InputIt first, InputIt last, OutputIt d_first);

// C++20ではコンセプトを用い要件を表現できます。　

template<std::input_iterator InputIt, std::output_iterator OutputIt>
OutputIt copy(InputIt first, InputIt last, OutputIt d_first);
```

```
std::copy()の引数は、コピー元を表す入力イテレータ2つと、コピー先を表す出力イテレータ1つです。入力イテレータと出力イテレータを明示的に要件とするのは、コピーしかしない以上当然です。これは引数に対する要件を最小化していると言えます。
```