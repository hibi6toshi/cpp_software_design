# テスト可用性に備え設計する

```
テストを用意しておく目的は、開発ソフトウェアが頻繁に発生する変更に耐え、全機能が以前と変わらず動作することを、確認可能にすることです。

またテストを作成するには、すなわち保護がその役割を果たせるようにするには、開発ソフトウェアがテスト可能になっていなければなりません。
理想を言えば、テストを**容易に追加可能**とすべきです。

ソフトウェアはテストに備えた設計とすべきです。
```

## privateメンバ関数のテスト方法
テストの考え方を理解するため、以下のクラスを考える。
Widgetは内部にBlobオブジェクトのvectorを保持しており、Blobを更新することがある。このため、WidgetにはupdateColection()メンバ関数があり、この関数をテストしたい。

```C++
class Widget {
  // ...
  private:
    void updateCollection(/* some arguments needed to update the collection */);

    std::vector<Blob> blobs_;
    /* potentially other data members */
};
```

updateCollection()メンバ関数はクラスのprivateに宣言されいて、外部から直接アクセスできず、直接にはテストできない。

1番簡単な方法は、別のpublicメンバ関数経由でupdateCollection()関数をコールすることです。

```C++
class Widget {
  public:
    // ...
    void addBlob(Blob const& blob, /* ... */) {
      // ...
      updateCollection(/* ... */);
      // ...
    }
  private:
    void updateCollection(/* some arguments needed to update the collection */);

    std::vector<Blob> blobs_;
    /* potentially other data members */  
};
```

このアプローチは、できるだけ避けるべきアプローチです。このようなテストを**ホワイトボックステスト**と言い、関数の内部実装まで把握した上で行うテストです。
このアプローチの問題点も、ソフトウェアの変更に弱いことです。
例えば、将来、addBlob()関数がupdateCollection()関数を呼ばないように変更された場合、updateCollection()のテストは実行されず、さらにそのことに気づかないかもしれません。

必要なのは**ブラックボックステスト**です。つまり、テスト対象の関数の内部実装になんら前提を設けずに、動作を確認するテストです。

privateを保ちながら、updateCollection()関数をテストするため、friendを作るアプローチを試す。
```C++
class Widget {
  // ...
  private:
    friend class WidgetTest;

    void updateCollection(/* some arguments needed to update the collection */);

    std::vector<Blob> blobs_;
    /* potentially other data members */
};
```

friendクラスからは、そのクラスの全てにアクセスできる。
そのため、Widgetクラスをテストするための部品として、WidgetTestクラスを作成し、Widgetクラスのfriendにするアプローチも妥当に思えるかもしれません。

しかし、これは設計という観点から見ると、再び人工的な依存関係を作り出したに過ぎません。friend宣言を加えたため、製品コードがテストコードを知った状態になり、テストコードも製品コードについて知ることになります。
製品コードは本来、テストコードについて何も知るべきではありません。

privateではなくて、protectedに変更し、テスト用の派生クラスを作成する方法を考えます。
```C++
class Widget {
  // ...
  protected:
    void updateCollection(/* some arguments needed to update the collection */);

    std::vector<Blob> blobs_;
    /* potentially other data members */
};

class TestWidget : public Widget {
  // ...
}
```
このアプローチは、継承の意味と適切な使い方を満たしていないため、避けるべきです。
非publicメンバ関数にアクセスするためだけに継承を使うのは、継承の意味を曲解していると言えます。

プリプロセッサを使って、privateを全部publicに変更する方法もある。
```C++
# define private public
class Widget {
  // ...
  private:
    void updateCollection(/* some arguments needed to update the collection */);

    std::vector<Blob> blobs_;
    /* potentially other data members */
};
```
このアプローチは、合理的な検討の範疇を超えています。プリプロセッサは、コードの可読性を損なうだけでなく、コンパイラのエラーメッセージも混乱させる可能性があります。

## 真の解: 関心の分離
クラスからprivateメンバ関数を切り出し、分離したソフトウェアエンティティとするアプローチを考えます。
これは、**ガイドライン2**で強調した、関心の分離の考え方に基づいています。

この例で言えば、メンバ関数をフリー関数（自由関数・外部関数）にします。
```C++
void updateCollection(std::vector<Blob>& blobs, /* some arguments needed to update the collection */);

class Widget {
  // ...
  private:
    std::vector<Blob> blobs_;
    /* potentially other data members */
};
```

上記を用いると、updateCollection()メンバ関数をコールしている箇所全てで、先頭引数にblobs_を加えるだけで、フリー関数のupdateCollection()のコールへ切り替えられます。
仮にこの関数が別の状態変数を使用しているならば、それを切り出し、別クラスとします。どちらの場合もコードはテストが容易であり、手間もかかりません。

```C++
namespace WidgetDetails {
  class BlobCollection {
    public:
      void updateCollection(/* some arguments needed to update the collection */);

    private:
      std::vector<Blob> blobs_;
  };
} // namespace WidgetDetails

class Widget {
  // ...
  private:
    widgetDetails::BlobCollection blobs;
    /* Other data members */
};
```

WidgetとupdateCollection()が同じものに属しているように見えるます。
そのように設計すると（Widgetのprivateメンバ関数としてupdateCollection()を持つと）updateCollection()のテストが困難になり、そのことが、その設計が問題であることを示しています。

```
updateCollection()関数が、個別のテストが必要なほど重要な場合、Widget以外の理由で更新するのは明白です。
この事実から、WidgetとupdateCollection()関数は同じものに属していないと判断できます。
そのため、SRPに従い、updateCollection()関数を別のクラスに分離することが適切です。
```

一方で、これはカプセル化に反していると考えられるかもしれません。
しかし、カプセル化は関心を分離する理由の1つです。クラスから関数を切り出すのはカプセル化を促進する一歩だと主張しています。
Scott Meyers(Effective C++)によれば、一般にメンバ関数よりも、メンバでもfriendでもない関数を優先すべきです。

これはメンバ関数は、クラス内の（privateメンバを含む）全メンバにフルアクセスできるためです。しかし、上例のようにupdateCollection関数を切り出すと、Widgetクラスのpublicインタフェースにしかアクセスできなくなり、privateメンバにはアクセスできなくなります。そのため、privateメンバのカプセル化が若干促進されます。

BlobCollectionクラスも同様に、Widgetクラスの非publicメンバにアクセスできず、カプセル化が促進されます。

関心を分離し機能を切り出したことで、
- Widgetクラスのカプセル化が促進された
- updateCollection()関数のテストが容易になった（テストにWidgetクラスを用いる必要はなく、先頭にblobs_渡すか、publicメンバ関数をよべばよい）
- Widgetクラスの他のアスペクトを変更する必要がありません。コレクション変更するときは、updateColeciton()関数へblobs_メンバを渡すだけでよく、ゲッターやセッターを追加する必要はありません。
- Widgetとは個別に関数を変更することができ、依存関係の削減につながる。（updateColllectionはそれ自体で再利用が可能）

全てのメンバ関数を切り出せというわけではなく、テストが必要なほど重要な関数をprivate部におくならば、それを切り出すことを検討すべきとうことです。
