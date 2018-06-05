require 'minitest/autorun'
require 'score_index/score_index'

class TestScore < Minitest::Test
  def test_score_index_trivial
    index = ScoreIndex.new(
      '2017-07-19T21:24:51Z localhost 443 NOPREFIX@ffffffffffffffff',
      6
    )
    assert_equal(index.value, '4034')
  end
end
